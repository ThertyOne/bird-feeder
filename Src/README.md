# Intelligent Bird Feeder ESP32-CAM

Oprogramowanie do projektu inteligentnego karmnika dla ptaków oparty o moduł ESP32-CAM.  
System łączy się z siecią Wi-Fi, komunikuje się z botem Telegrama i docelowo ma wykonywać zdjęcia oraz rozpoznany gatunek, po wykryciu obecności ptaka.

## Funkcjonalności

Aktualnie zaimplementowane:

- konfiguracja projektu przez osobny plik `config.h`,
- połączenie ESP32-CAM z siecią Wi-Fi,
- podstawowa struktura modułowa kodu,
- przygotowany moduł komunikacji z Telegram Bot API.

Planowane / rozwijane:

- obsługa komend Telegrama,
- inicjalizacja kamery OV2640,
- wykonywanie zdjęcia,
- wysyłanie zdjęcia przez Telegram,
- detekcja ptaka z użyciem czujnika ruchu / odległości,
- rozpoznawanie gatunku ptaka.

## Struktura projektu

```txt
src/
├── main.cpp
├── config.h                    # lokalny plik konfiguracyjny, nie obecny w repo
├── config.example.h            # przykładowa konfiguracja
├── sensors_n_actuators/
│     ├── wifi_manager.h
│     ├── wifi_manager.cpp
│     └── ...
├── telegram/
│     ├── telegram.h
│     └── telegram.cpp
```