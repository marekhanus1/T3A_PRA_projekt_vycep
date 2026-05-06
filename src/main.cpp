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
      const char* label = jeMaly ? "0,5 l" : "0,3 l";
      
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
  
  // Horní řádek: Název nápoje
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, 15);
  u8g2.print(nazev);
  
  // Velikost nápoje (menším písmem)
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(85, 15);
  u8g2.print(velikost);
  
  // Progress bar
  u8g2.drawFrame(0, 30, 128, 12);
  u8g2.drawBox(2, 32, map(proc, 0, 100, 0, 124), 8);
  
  // Procenta
  u8g2.setCursor(55, 60);
  u8g2.print(proc);
  u8g2.print("%");
  
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