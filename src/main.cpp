#include "config.h"

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RES); // definice OLED displeje

Servo ventil; // definice serva pro ventil

Napoj menu[3] = { // Struktura pro nápoje
  {"Pivo", CAP_PIVO, 0},
  {"Birell", CAP_BIRELL, 0},
  {"Kofola", CAP_KOFOLA, 0}
};


void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.enableUTF8Print(); // Povolení diakritiky
  ventil.attach(SERVO_PIN);
  ventil.write(SERVO_ZAVRENO);

  for (int i = 0; i < 6; i++) {
    pinMode(PINY_TLACITEK[i], INPUT_PULLUP);
  }
}

void loop() {
  obsluhaTlacitek();
}

void obsluhaTlacitek() {
  for (int i = 0; i < 6; i++) {
    if (digitalRead(PINY_TLACITEK[i]) == LOW) {
      int indexNapoje = i / 2;
      bool jeMaly = (i % 2 == 0);
      int objem = jeMaly ? OBJEM_M : OBJEM_V;
      const char* label = jeMaly ? "0,3 l" : "0,5 l";
      
      provadejVydej(indexNapoje, objem, label);
    }
  }
}

void provadejVydej(int index, int objem, const char* labelVelikost) {
  if (menu[index].zbyva < objem) {
    chybaNedostatek(menu[index].nazev);
    return;
  }

  otevriVentil();
  long dobaVydaje = (objem / (float)RYCHLOST_CEPOVANI) * 1000;
  long start = millis();
  
  while (millis() - start < dobaVydaje) {
    int procenta = map(millis() - start, 0, dobaVydaje, 0, 100);
    zobrazStatus(menu[index].nazev, labelVelikost, procenta);
  }

  zavriVentil();
  aktualizujStatistiky(index, objem);
}

void zobrazStatus(const char* nazev, const char* velikost, int proc) {
  u8g2.clearBuffer();
  
  // 1. STATICKÝ TEXT (Název a velikost)
  u8g2.setFont(u8g2_font_6x10_tf); // Úsporný font
  u8g2.setCursor(0, 10);
  u8g2.print(nazev);
  
  u8g2.setCursor(95, 10);
  u8g2.print(velikost);
  
  // Horizontální oddělovací čára pod textem (U8g2 používá drawHLine)
  u8g2.drawHLine(0, 14, 128);

  // 2. DIGITÁLNÍ TORRENT ANIMACE (Matrix proud)
  int casovyPosun = millis() / 15; // Rychlost padání proudu
  
  // Vykreslíme 14 vertikálních proudů (osa X: 0 až 80)
  for (int i = 0; i < 14; i++) {
    int x = i * 6; 
    int startY = (casovyPosun + (i * 17)) % 65; 
    int delkaCary = 8 + (i % 3) * 5;            
    
    if (startY > 16 && startY < 64) {
      int vykreslitDelku = (startY + delkaCary > 64) ? (64 - startY) : delkaCary;
      
      // Opraveno na drawVLine
      u8g2.drawVLine(x, startY, vykreslitDelku);
      
      // Jiskra na konci čáry pro lepší Matrix efekt
      if (startY + vykreslitDelku < 63) {
        u8g2.drawPixel(x + 1, startY + vykreslitDelku);
      }
    }
  }

  // 3. ADAPTIVNÍ UKAZATEL PROCENT (Pravá strana displeje)
  // Opraveno na drawVLine
  u8g2.drawVLine(90, 15, 49);
  
  // Velký ukazatel procent
  u8g2.setFont(u8g2_font_helvB14_tr);
  u8g2.setCursor(95, 40);
  u8g2.print(proc);
  
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.print("%");
  
  // Mini doplňkový progress bar na boku
  u8g2.drawFrame(95, 48, 28, 8);
  u8g2.drawBox(97, 50, map(proc, 0, 100, 0, 24), 4);
  
  u8g2.sendBuffer();
}

// Ostatní funkce zůstávají stejné...
void otevriVentil() { ventil.write(SERVO_OTEVRENO); }
void zavriVentil() { ventil.write(SERVO_ZAVRENO); u8g2.clearDisplay(); }

void aktualizujStatistiky(int index, int objem) {
  menu[index].zbyva -= objem;
  menu[index].vytoceno += objem;
  if (menu[index].zbyva < VAROVANI_LIMIT) {
    Serial.print(F("VAROVANI: Nizky stav - "));
    Serial.print(menu[index].nazev);
    Serial.print(F(" ("));
    Serial.print(menu[index].zbyva);
    Serial.println(F(" ml)"));
  }
}

void chybaNedostatek(const char* nazev) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 35, "Nedostatek: ");
  u8g2.setCursor(85, 35);
  u8g2.print(nazev);
  u8g2.sendBuffer();
  delay(2000);
}
