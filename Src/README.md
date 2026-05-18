# Intelligent Bird Feeder ESP32-CAM

Oprogramowanie do projektu inteligentnego karmnika dla ptaków oparty o moduł ESP32-CAM.  
System łączy się z siecią Wi-Fi, komunikuje się z botem Telegrama i docelowo ma wykonywać zdjęcia oraz rozpoznany gatunek, po wykryciu obecności ptaka.

## Funkcjonalności

Aktualnie zaimplementowane:

- konfiguracja projektu przez osobny plik `config.h`,
- połączenie ESP32-CAM z siecią Wi-Fi,
- inicjalizacja kamery OV2640,
- wykonywanie zdjęcia,
- wysyłanie zdjęcia przez Telegram,
- podstawowa struktura modułowa kodu,
- detekcja ptaka z użyciem czujnika ruchu / odległości,
- obsługa komend Telegrama,
- przygotowany moduł komunikacji z Telegram Bot API.

Planowane / rozwijane:

- rozpoznawanie gatunku ptaka.

## Struktura projektu

```txt
src/
├── main.cpp
├── config.h                    # lokalny plik konfiguracyjny, nie obecny w repo
├── config.example.h            # przykładowa konfiguracja  
|     
├── sensors_n_actuators/        # moduły fizyczne
│     ├── camera/
│     |     ├── camera.h
│     |     └── camer.cpp
|     |
│     ├── pir_sensor/
│     |     ├── pir_sensor.h
│     |     └── pir_sensor.cpp
|     |
│     ├── wifi_meneger/
│           ├── wifi_meneger.h
│           └── wifi_meneger.cpp
|
└── telegram/                   # serwer Telegram
      ├── telegram.h
      └── telegram.cpp
```

## Konfiguracja
Utwórz plik `src/config.h` na podstawie `src/config.example.h`:
```c
#define SSID_1 "NAZWA_WIFI"
#define PASSWORD_1 "HASLO_WIFI"

#define BOT_TOKEN "TOKEN_BOTA_TELEGRAM"
#define CHAT_ID "ID_CZATU"

#define PIR_SENSOR_PIN 14
#define FLASH_LED_PIN 4
#define SERVO_PIN 15
```

## Uruchomienie debugowania
1. Skonfiguruj `config.h`.
2. Podłącz `ESP32-CAM` do stabilnego zasilania 5 V.
3. Wgraj program na płytkę.
4. Otwórz Serial Monitor z prędkością: `115200 baud`.
5. Sprawdź, czy urządzenie poprawnie łączy się z Wi-Fi.


## Docs
### Telegram:
Moduł `TelegramManager` odpowiada za komunikację ESP32-CAM z Telegram Bot API.

Obsługuje:
- wysyłanie wiadomości tekstowych,
- wysyłanie zdjęć JPEG,
- odbieranie prostych komend użytkownika.

## Wymagania konfiguracyjne

W pliku `config.h` muszą być zdefiniowane:

```cpp
#define BOT_TOKEN "telegram_bot_token"
#define CHAT_ID "authorized_chat_id"
#define BOT_REQUEST_DELAY 1000UL