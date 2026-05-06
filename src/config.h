#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Servo.h>
#include <SPI.h>

// --- HARDWARE PINY ---
#define OLED_CS     10
#define OLED_DC     9
#define OLED_RES    8
#define SERVO_PIN   6

const int PINY_TLACITEK[6] = {2, A0, 3, A1, 4, A2}; // Pivo(M,V), Birell(M,V), Kofola(M,V)

// --- KONSTANTY NÁPOJŮ ---
#define CAP_PIVO        50000 // 50 litrů v ml
#define CAP_BIRELL      30000
#define CAP_KOFOLA      50000
#define VAROVANI_LIMIT  5000

#define OBJEM_M         300 // 300 ml
#define OBJEM_V         500 // 500 ml
#define RYCHLOST_CEPOVANI 100 // 100 ml/s

#define SERVO_ZAVRENO   0
#define SERVO_OTEVRENO  90

// --- STRUKTURY ---
struct Napoj {
  const char* nazev;
  long zbyva;
  long vytoceno;
};

// --- GLOBÁLNÍ OBJEKTY (Externí deklarace) ---
extern U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2;
extern Servo ventil;
extern Napoj menu[3];

// --- PROTOTYPY FUNKCÍ (Řeší chybu "not declared") ---
void obsluhaTlacitek();
void provadejVydej(int index, int objem, const char* labelVelikost);
void zobrazStatus(const char* nazev, const char* velikost, int proc);
void otevriVentil();
void zavriVentil();
void aktualizujStatistiky(int index, int objem);
void chybaNedostatek(const char* nazev);

#endif