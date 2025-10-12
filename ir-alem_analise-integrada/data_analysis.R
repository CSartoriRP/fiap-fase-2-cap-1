# analysis.R
# Lê logs do Wokwi (logs.txt) + weather.csv (opcional) e gera base limpa + gráficos.

# ---------- Pacotes ----------
req <- c("readr","dplyr","stringr","ggplot2","tidyr","scales","corrplot")
new <- req[!(req %in% installed.packages()[,"Package"])]
if(length(new)) install.packages(new, repos="https://cloud.r-project.org")
invisible(lapply(req, library, character.only=TRUE))

# ---------- Entradas ----------
logs_file    <- "logs.txt"      # copie o Serial do Wokwi para este arquivo
weather_file <- "weather.csv"   # opcional; se não existir, seguimos só com os logs
dir.create("plots", showWarnings = FALSE)

stopifnot(file.exists(logs_file))
logs <- read_lines(logs_file)

# ---------- Helpers ----------
num <- function(x) suppressWarnings(as.numeric(x))
as01 <- function(x) ifelse(x %in% c("ON","1","TRUE","T"), 1, 0)

# Regex para dois formatos:
# 1) Linha contínua (sem [EVT]), ex:
# N=8.3 P=10.0 K=8.1 | pH=6.34 (5.50-6.60) | Umid=19.3% | Nutrientes: OK | Ext[POP=70.0% RAIN3H=NAmm HOLD=ON] | Bomba=OFF
pat_cont <- paste0(
  "N=([0-9\\.]+)\\s+P=([0-9\\.]+)\\s+K=([0-9\\.]+).*?",
  "pH=([0-9\\.]+).*?\\|\\s*Umid=([0-9\\.]+)%.*?",
  "Ext\\[POP=([0-9\\.NA]+)%\\s+RAIN3H=([0-9\\.NA]+)mm\\s+HOLD=([A-Z]+)\\].*?",
  "\\|\\s*Bomba=(ON|OFF)"
)

# 2) Linha de evento (se você habilitou o patch [EVT]), ex:
# [EVT] Bomba=ON Motivo=CHUVA POP=70.0% RAIN3H=0.00mm Umid=32.6%
pat_evt <- "\\[EVT\\].*?Bomba=(ON|OFF).*?Motivo=([A-Z_]+).*?POP=([0-9\\.NA]+)%.*?RAIN3H=([0-9\\.NA]+)mm.*?Umid=([0-9\\.]+)%"

# ---------- Parse ----------
parse_cont <- logs[str_detect(logs, "N=") & str_detect(logs, "Bomba=")]
m <- str_match(parse_cont, pat_cont)
df_cont <- tibble(
  idx = which(logs %in% parse_cont),
  N = num(m[,2]),
  P = num(m[,3]),
  K = num(m[,4]),
  pH = num(m[,5]),
  Umid = num(m[,6]),
  POP = num(str_replace_na(m[,7], "NA")),
  RAIN3H = num(str_replace_na(m[,8], "NA")),
  HOLD = m[,9],
  Bomba = m[,10]
)

parse_evt <- logs[str_detect(logs, "^\\[EVT\\]")]
m2 <- str_match(parse_evt, pat_evt)
df_evt <- tibble(
  idx = which(logs %in% parse_evt),
  N = NA_real_, P = NA_real_, K = NA_real_, pH = NA_real_,
  Umid = num(m2[,6]),
  POP = num(str_replace_na(m2[,4], "NA")),
  RAIN3H = num(str_replace_na(m2[,5], "NA")),
  HOLD = NA_character_,
  Bomba = m2[,2],
  Motivo = m2[,3]
)

# Une bases (mantém a ordem de aparecimento via idx)
events <- bind_rows(df_cont, df_evt) |>
  arrange(idx) |>
  mutate(
    Bomba_on = if_else(Bomba == "ON", 1L, 0L),
    HOLD_on  = if_else(HOLD == "ON", 1L, 0L),
    # NA -> valores seguros para análise
    POP = replace_na(POP, 0),
    RAIN3H = replace_na(RAIN3H, 0),
    Umid = replace_na(Umid, NA_real_)
  )

# Salva base limpa principal
write_csv(events, "irrigation_events.csv")
message("OK: irrigation_events.csv gerado.")

# ---------- (Opcional) Conciliação com weather.csv ----------
if (file.exists(weather_file)) {
  weather <- suppressWarnings(readr::read_csv(weather_file, show_col_types = FALSE))

  # Normaliza nomes possíveis
  nm <- names(weather)
  has_POP <- "POP" %in% nm
  has_pop <- "pop" %in% nm
  has_rain3h_lower <- "rain_3h" %in% nm
  has_rain3h_upper <- "RAIN3H" %in% nm

  weather <- weather |>
    dplyr::mutate(
      pop_norm = dplyr::coalesce(!!dplyr::sym(if (has_POP) "POP" else if (has_pop) "pop" else NULL), 0),
      rain3h_norm = dplyr::coalesce(!!dplyr::sym(if (has_rain3h_lower) "rain_3h" else if (has_rain3h_upper) "RAIN3H" else NULL), 0)
    )

  weather_last <- weather |>
    dplyr::group_by(city_query) |>
    dplyr::slice_tail(n = 1) |>
    dplyr::ungroup() |>
    dplyr::select(city_query, temp_c, humidity, rain_3h = rain3h_norm, pop = pop_norm)

  readr::write_csv(weather_last, "weather_latest_by_city.csv")
  message("OK: weather_latest_by_city.csv gerado (amostra de referência).")
}


# ---------- Gráficos ----------
# 1) Histerese vs Umidade
p1 <- events |>
  filter(!is.na(Umid)) |>
  ggplot(aes(x = Umid, y = Bomba_on)) +
  geom_jitter(height = 0.05, alpha = 0.6) +
  geom_vline(xintercept = 55, linetype = 2) +
  geom_vline(xintercept = 70, linetype = 2) +
  scale_y_continuous(breaks = c(0,1), labels = c("OFF","ON")) +
  labs(title = "Acionamento da bomba vs Umidade",
       x = "Umidade do solo (%)", y = "Bomba") +
  theme_minimal()
ggsave("plots/bomba_vs_umidade.png", p1, width = 8, height = 4, dpi = 150)

# 2) Influência POP e RAIN3H
p2 <- events |>
  mutate(POPc = pmin(POP,100)) |>
  ggplot(aes(x = POPc, y = RAIN3H, color = factor(Bomba_on))) +
  geom_point(alpha = 0.7) +
  scale_color_manual(values = c("0"="#777777","1"="#1f77b4"), labels=c("OFF","ON"), name="Bomba") +
  labs(title = "Bomba vs POP (prob. de chuva) e RAIN3H (mm)",
       x = "POP (%)", y = "RAIN3H (mm/3h)") +
  theme_minimal()
ggsave("plots/bomba_vs_pop_rain.png", p2, width = 8, height = 4, dpi = 150)

# 3) Correlação (quando houver dados suficientes)
num_cols <- events |>
  select(N, P, K, pH, Umid, POP, RAIN3H, Bomba_on) |>
  mutate(across(everything(), as.numeric))
corr <- suppressWarnings(cor(num_cols, use="pairwise.complete.obs"))
png("plots/correlacao.png", width = 800, height = 600)
corrplot::corrplot(corr, method="color", type="upper", addCoef.col="black", tl.col="black")
dev.off()

# ---------- Modelo simples (logístico): Bomba_on ~ Umid + POP + RAIN3H ----------
mod_ok <- nrow(na.omit(select(events, Bomba_on, Umid, POP, RAIN3H))) > 10
if (mod_ok) {
  dfm <- na.omit(select(events, Bomba_on, Umid, POP, RAIN3H))
  fit <- glm(Bomba_on ~ Umid + POP + RAIN3H, data=dfm, family=binomial())
  sink("modelo_logistico.txt"); print(summary(fit)); sink()
  message("OK: modelo_logistico.txt gerado.")
} else {
  message("Aviso: poucos dados úteis para ajustar o modelo logístico.")
}

message("Concluído. Veja a pasta 'plots/' e os arquivos CSV gerados.")
