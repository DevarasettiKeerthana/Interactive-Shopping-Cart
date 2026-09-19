#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

/* ---------- FUNCTION PROTOTYPES ---------- */
void showItem(String name, int price);
void payment(String name);
void showIdleScreen();

/* ---------- PIN DEFINITIONS ---------- */
#define SS_PIN     10
#define RST_PIN    A3
#define BUZZER     A2
#define BUTTON     8

SoftwareSerial BT(A1, A0);   // RX, TX
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
MFRC522 mfrc522(SS_PIN, RST_PIN);

/* ---------- ITEM COST ---------- */
const int oil  = 100;
const int pen  = 20;
const int soap = 50;

/* ---------- VARIABLES ---------- */
int o = 0, p = 0, s = 0;
int cost = 0;
int amount = 1500;

/* ---------- RFID UIDs ---------- */
#define UID_SOAP  "41F58B55"
#define UID_OIL   "23F5643D"
#define UID_PEN   "2057E02F"
#define UID_PAY1  "F315030D"
#define UID_PAY2  "092674B3"

/* ---------- LCD STATE ---------- */
bool idleScreenShown = false;

/* ---------- BUZZER ---------- */
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(60);
  digitalWrite(BUZZER, LOW);
}

/* ---------- LCD HELPER ---------- */
void lcdstring(int c, int r, String data) {
  lcd.setCursor(c, r);
  lcd.print(data);
}

/* ---------- IDLE SCREEN ---------- */
void showIdleScreen() {
  lcd.clear();
  lcdstring(0,0,"Please show card");
  lcdstring(0,1,"                ");
  idleScreenShown = true;
}

/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("SMART TROLLEY STARTED");
  Serial.println("BUTTON LOW  -> REMOVE MODE");
  Serial.println("BUTTON HIGH -> ADD MODE");

  lcdstring(0,0,"SMART TROLLY");
  lcdstring(0,1,"SHOW CARD");
  delay(2000);

  showIdleScreen();
}

/* ---------- LOOP ---------- */
void loop() {

  if (!idleScreenShown) showIdleScreen();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  idleScreenShown = false;
  beep();

  /* ----- READ UID ----- */
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.print("UID Detected: ");
  Serial.println(uid);

  bool removeMode = (digitalRead(BUTTON) == LOW);
  Serial.println(removeMode ? "MODE: REMOVE" : "MODE: ADD");

  /* ================= OIL ================= */
  if (uid == UID_OIL) {

    if (removeMode) {
      if (o > 0) {
        o--;
        cost -= oil;
        Serial.println("you REMOVED -- OIL = 100");
        BT.println("REMOVED: OIL = 100");
      } else {
        Serial.println("OIL already ZERO");
        BT.println("OIL already ZERO");
        lcd.clear();
        lcdstring(0,0,"OIL = 0");
        lcdstring(0,1,"NOTHING TO REMOVE");
        delay(1200);
        return;
      }
    } else {
      o++;
      cost += oil;
      Serial.println("you enterred -- OIL = 100");
      BT.println("ADDED: OIL = 100");
    }

    if (cost < 0) cost = 0;
    showItem("OIL", oil);
  }

  /* ================= PEN ================= */
  else if (uid == UID_PEN) {

    if (removeMode) {
      if (p > 0) {
        p--;
        cost -= pen;
        Serial.println("you REMOVED -- PEN = 20");
        BT.println("REMOVED: PEN = 20");
      } else {
        Serial.println("PEN already ZERO");
        BT.println("PEN already ZERO");
        lcd.clear();
        lcdstring(0,0,"PEN = 0");
        lcdstring(0,1,"NOTHING TO REMOVE");
        delay(1200);
        return;
      }
    } else {
      p++;
      cost += pen;
      Serial.println("you enterred -- PEN = 20");
      BT.println("ADDED: PEN = 20");
    }

    if (cost < 0) cost = 0;
    showItem("PEN", pen);
  }

  /* ================= SOAP ================= */
  else if (uid == UID_SOAP) {

    if (removeMode) {
      if (s > 0) {
        s--;
        cost -= soap;
        Serial.println("you REMOVED -- SOAP = 50");
        BT.println("REMOVED: SOAP = 50");
      } else {
        Serial.println("SOAP already ZERO");
        BT.println("SOAP already ZERO");
        lcd.clear();
        lcdstring(0,0,"SOAP = 0");
        lcdstring(0,1,"NOTHING TO REMOVE");
        delay(1200);
        return;
      }
    } else {
      s++;
      cost += soap;
      Serial.println("you enterred -- SOAP = 50");
      BT.println("ADDED: SOAP = 50");
    }

    if (cost < 0) cost = 0;
    showItem("SOAP", soap);
  }

  /* ================= PAYMENT ================= */
  else if (uid == UID_PAY1) {
    payment("Keerthana");
  }
  else if (uid == UID_PAY2) {
    payment("Harshini");
  }
  else {
    Serial.println("INVALID CARD");
    lcd.clear();
    lcdstring(0,0,"INVALID CARD");
    beep();
    delay(1200);
  }

  delay(600);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

/* ---------- SHOW ITEM ---------- */
void showItem(String name, int price) {
  lcd.clear();
  lcdstring(0,0,name + " : " + String(price));
  lcdstring(0,1,"TOTAL=" + String(cost));
  delay(1500);
}

/* ---------- PAYMENT ---------- */
void payment(String name) {

  if (cost == 0) {
    Serial.println("PAYMENT BLOCKED: CART EMPTY");
    BT.println("CART IS EMPTY");
    BT.println("ADD ITEMS FIRST");
    lcd.clear();
    lcdstring(0,0,"CART IS EMPTY");
    lcdstring(0,1,"ADD ITEMS");
    delay(2000);
    return;
  }

  lcd.clear();
  lcdstring(0,0,"CARD AUTHORIZED");
  delay(1200);

  lcd.clear();
  lcdstring(0,0,"FINAL BILL:");
  lcdstring(12,0,String(cost));
  delay(1200);

  if (amount < cost) {
    Serial.println("LOW BALANCE");
    lcd.clear();
    lcdstring(0,0,"LOW BALANCE");
    delay(2000);
    return;
  }

  amount -= cost;

  BT.println("Card Holder: " + name);
  if (o > 0) BT.println("Oil  x" + String(o) + " = " + String(o * oil));
  if (p > 0) BT.println("Pen  x" + String(p) + " = " + String(p * pen));
  if (s > 0) BT.println("Soap x" + String(s) + " = " + String(s * soap));
  BT.println("Pay Success: " + String(cost));
  BT.println("Remaining Bal: " + String(amount));
  BT.println();

  Serial.println("PAYMENT SUCCESS");

  lcd.clear();
  lcdstring(0,0,"PAYMENT OK");
  lcdstring(0,1,"BAL=" + String(amount));
  delay(2000);

  o = p = s = 0;
  cost = 0;
}