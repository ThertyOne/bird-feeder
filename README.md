# 🐦 Solar Bird Feeder – Smart Camera Feeder Project

## 📘 Opis projektu
Projekt **inteligentnego karmnika dla ptaków** zasilanego energią słoneczną.  
Urządzenie wykonuje zdjęcia odwiedzających ptaków, zapisuje je lokalnie lub przesyła do serwera,  
a następnie może uruchamiać mechanizm podania karmy.  
Projekt został zaprojektowany w konwencji **low-cost, low-power** z możliwością działania off-grid.

---

## ⚙️ Główne założenia
- Zasilanie autonomiczne: **panel solarny + akumulator Li-Po 3.7 V**
- Mikrokontroler i kamera: **ESP32-CAM (OV2640)**  
- Komunikacja: **Wi-Fi**, z opcjonalną obsługą LoRaWAN (SX1276 / SX1262)
- Peryferia:
  - **ultradźwiękowy** – detekcja obecności ptaka  
  - **serwo** – podanie karmy  
  - **pierścień LED WS2812** – doświetlenie sceny podczas zdjęcia
- Deep-sleep ESP32: wybudzenie po przerwaniu z czujnika
- Przesyłanie zdjęcia i metadanych (data, godzina) do serwera lub lokalnej bazy danych

---
