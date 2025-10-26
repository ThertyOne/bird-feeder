# 🐦 Solar Bird Feeder – Smart Camera Feeder Project

## 📘 Opis projektu
Projekt to **autonomiczny karmnik dla ptaków z funkcją wykonywania zdjęć**, oparty na module **ESP32-CAM**, zasilany z akumulatora **Li-Po 3,7 V / 2200 mAh**, ładowanego przez ładowarkę **TP4056** z panelem solarnym **6 V / 1 W** (możliwość połączenia 2 paneli szeregowo).  

Z wyjścia baterii energia trafia do **przetwornicy boost 5 V**, zbudowanej od podstaw na bazie generatora **PWM LMC555** i MOSFETa **IRLML6344**, która zasila ESP32 i peryferia – **serwo sterujące karmnikiem, LEDy oświetleniowe oraz czujniki**. Masa wszystkich modułów jest wspólna.  

Ładowanie z panelu odbywa się przez **diodę SS34** i **polyfuse 500 mA**, a napięcie akumulatora jest monitorowane przez dzielnik rezystorowy podłączony do ADC ESP.  

Po wykryciu ruchu przez czujnik ESP wychodzi z trybu **deep sleep**, włącza LED, wykonuje zdjęcie i wysyła je przez Wi-Fi na serwer, po czym ponownie przechodzi w uśpienie, oszczędzając energię. **Peryferia** (serwo, LEDy) są zasilane poprzez **klucz MOSFET**, który odcina zasilanie, gdy peryferia nie działają.

---

## ⚙️ Główne założenia
- Zasilanie autonomiczne: **panel solarny + akumulator Li-Po 3.7 V / 2200 mAh**
- Mikrokontroler i kamera: **ESP32-CAM (OV2640)**
- Komunikacja: **Wi-Fi**, opcjonalnie **LoRaWAN** w przyszłości
- Peryferia:
  - **Ultradźwiękowy czujnik / PIR** – detekcja obecności ptaka
  - **Serwo** – mechanizm podawania karmy
  - **Blue LED matrix 4x4** – doświetlenie sceny podczas zdjęcia
- ESP32 w trybie **deep sleep**, wybudzanie po przerwaniu z czujnika
- Przesyłanie zdjęcia i metadanych (data, godzina) do serwera lub lokalnej bazy danych

---

## ⚡ Przetwornica boost 5 V
- Źródło PWM: **LMC555 @ ~100 kHz, 3.3 V**
- MOSFET: **IRLML2502 / IRLML6344**
- Cewka: **15 μH**
- Dioda: **SS34**
- Kondensator wyjściowy: **4700 μF**
- Ograniczenie napięcia: **Zener 5 V**
- Funkcja: podnosi napięcie z 3,7 V akumulatora Li-Po do stabilnych 5 V dla ESP32 i peryferiów

---

## 🔋 Szacowane zużycie prądu
| Moduł            | Tryb/obciążenie             | Prąd (mA) |
|-----------------|----------------------------|------------|
| ESP32-CAM       | Deep sleep                 | 0.1 – 0.2 |
| ESP32-CAM       | Aktywne (Wi-Fi + foto)     | 180 – 250 |
| Serwo           | Ruch                        | 150 – 200 |
| LEDy WS2812     | Max 8 LED                   | 80 – 100 |
| Czujniki        | Idle                        | 2 – 5      |

**Średni pobór prądu:** ~10–30 mA przy większości czasu w deep sleep.  
**Maksymalny chwilowy pobór:** ~500 mA przy jednoczesnym ruchu serwa i LED.

---

## 💡 Sposoby oszczędzania energii
- ESP32 w **deep sleep**, wybudzenie tylko przy wykryciu ruchu
- Peryferia (LED, serwo) zasilane przez **MOSFET**, odcięcie zasilania gdy nieaktywne
- Minimalizacja czasu pracy Wi-Fi i kamer
- Możliwość zmniejszenia jasności LED i liczby LED w pierścieniu
- Optymalizacja częstotliwości wybudzeń w zależności od natężenia odwiedzin ptaków

---

## ☀️ Energia z baterii i paneli
- **Akumulator Li-Po:** 3,7 V / 2200 mAh → **8,14 Wh**
- **Panel solarny:** 6 V / 1 W → maksymalny prąd ładowania ~167 mA (przy pełnym słońcu)
- **2 panele szeregowo:** napięcie 12 V / prąd ~83 mA, dalej prostownik i TP4056
- **Przetwornica boost 5 V:** sprawność ~85 % → prąd pobierany z akumulatora = Iout × 5 V / (η × 3,7 V)
- **Szacowany średni pobór prądu ESP32 + peryferia:** 50 mA @ 5 V  
  \[
  I_{bat} \approx \frac{5V \cdot 0.05A}{0.85 \cdot 3.7V} \approx 79,5\, mA
  \]
- **Czas pracy na baterii bez słońca:**  
  \[
  t \approx \frac{2200 mAh}{79,5 mA} \approx 27,7 h \approx 1,15 dnia
  \]
- **Chwilowe piki prądu** (serwo + LED + Wi-Fi): ~500 mA @ 5 V → Ibat_peak ≈ 800 mA  
- **Przy słonecznej pogodzie** panele mogą praktycznie utrzymać akumulator w pełnym stanie, a nawet doładować go na zapas (np. 4–5 h słońca → ~4 Wh energii)


---

## 🔧 Uwagi do przetwornicy
- Układ **boost 5 V** przy chwilowych pikach prądu (serwo + LED + Wi-Fi) mogą wystąpić spadki napięcia
- Kondensator 4700 μF pomaga w wygładzaniu napięcia przy skokach prądowych
- Zener 5 V chroni ESP32 przed przepięciem, ale warto monitorować temperaturę MOSFETa i cewki przy pełnym obciążeniu
- Ewentualne zwiększenie cewki lub dodanie małego kondensatora ceramicznego przy MOSFET zwiększy stabilność przy dużym poborze

---
