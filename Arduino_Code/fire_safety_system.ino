#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// ---------- GPS ----------
TinyGPSPlus gps;

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- DHT11 ----------
#define DHTPIN 10
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- GSM & GPS Serial ----------
SoftwareSerial gsm(7, 8);       // GSM RX, TX
SoftwareSerial gpsSerial(4, 3); // GPS RX, TX

// ---------- Sensors ----------
#define fire A1

// ---------- Outputs ----------
#define buzzer 11
#define pump 6
#define lock 12
#define relay 9
#define light A2

// ---------- Variables ----------
int fireStatus;
float temperature;
float humidity;

// ---------- Default Location ----------
String latitude = "28.4639";
String longitude = "77.4987";

// ---------- GPS Validation ----------
int gpsValidCount = 0;
const int gpsCheckLimit = 10;

void setup()
{
  Serial.begin(9600);

  gsm.begin(9600);
  gpsSerial.begin(9600);

  // ---------- Pin Modes ----------
  pinMode(buzzer, OUTPUT);
  pinMode(lock, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(light, OUTPUT);

  pinMode(fire, INPUT);

  // ---------- LCD ----------
  lcd.init();
  lcd.backlight();

  // ---------- DHT ----------
  dht.begin();

  // ---------- Startup Message ----------
  lcd.setCursor(0, 0);
  lcd.print("SMART FIRE");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM");

  delay(3000);
  lcd.clear();
}

void loop()
{
  // ---------- GPS SECTION ----------

  gpsSerial.listen();

  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid())
  {
    gpsValidCount++;

    if (gpsValidCount >= gpsCheckLimit)
    {
      latitude = String(gps.location.lat(), 6);
      longitude = String(gps.location.lng(), 6);

      lcd.setCursor(0, 1);
      lcd.print("GPS LOCKED   ");
    }
  }
  else
  {
    latitude = "28.4639";
    longitude = "77.4987";

    lcd.setCursor(0, 1);
    lcd.print("GPS DEFAULT  ");
  }

  // ---------- DHT11 READ ----------

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  lcd.setCursor(0, 0);

  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C ");

  lcd.print("H:");
  lcd.print(humidity);
  lcd.print("% ");

  // ---------- TEMPERATURE ALERT ----------

  if (temperature >= 50)
  {
    digitalWrite(buzzer, HIGH);
    delay(300);

    digitalWrite(buzzer, LOW);
    delay(300);
  }

  // ---------- FIRE SENSOR ----------

  fireStatus = digitalRead(fire);

  if (fireStatus == LOW)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("FIRE ALERT!");

    digitalWrite(buzzer, HIGH);
    digitalWrite(pump, HIGH);
    digitalWrite(lock, HIGH);
    digitalWrite(relay, LOW);

    sendAlert("Fire Accident Alert!");

    delay(8000);
  }
  else
  {
    digitalWrite(pump, LOW);
    digitalWrite(lock, LOW);
    digitalWrite(relay, HIGH);
    digitalWrite(buzzer, LOW);
  }

  delay(1000);
}

// ---------- SMS FUNCTIONS ----------

void init_sms(String number)
{
  gsm.listen();

  gsm.println("AT+CMGF=1");
  delay(1000);

  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");

  delay(1000);
}

void send_sms(String number, String message)
{
  gsm.listen();

  init_sms(number);

  gsm.print(message);

  delay(500);

  gsm.write(26);

  delay(5000);
}

// ---------- ALERT FUNCTION ----------

void sendAlert(String alertType)
{
  String message = alertType;

  message += "\nLocation:\n";
  message += "https://maps.google.com/?q=";
  message += latitude;
  message += ",";
  message += longitude;

  // ---------- Emergency Number ----------
  send_sms("+91XXXXXXXXXX", message);

  Serial.println(message);
}