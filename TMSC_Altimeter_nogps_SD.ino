#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_SH110X.h>
#include <SD.h>
#include <RTCZero.h>



Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

Adafruit_BMP3XX bmp;

File logfile;

RTCZero rtc;

#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
#define cardSelect 4

uint8_t i = 0;

float altstart = 0;
float altagl = 0;
float altft = 0;
float tempc = 0;
float tempf = 0;
int R = 0;
/* Change these values to set the current initial time */
const byte hours = 0;
const byte minutes = 0;
const byte seconds = 0;
/* Change these values to set the current initial date */
const byte day = 10;
const byte month = 11;
const byte year = 21;
String RTCtime = ("");

void setup() {

  Serial.begin(115200);

  R = 0;

  rtc.begin();
  //rtc.setTime(hours, minutes, seconds);   // Set the time
  //rtc.setDate(day, month, year);    // Set the date

  //display setup
  display.begin(0x3C, true); // Address 0x3C default
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // Boot message
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("TMSC Alt/Temp Module ");
  display.println("Ver 1.0");
  display.println("Developed by ZED");
  display.println("Don't forget to tap!");
  display.println("");
  display.display();



  //Altimeter setup
  float SEALEVELPRESSURE = (1013.25);

  if (!bmp.begin_I2C()) {   // hardware I2C mode, can pass in address & alt Wire
    display.println("Could not find a valid BMP3 sensor, check wiring!");
    display.display();
    while (1);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    display.println("Card init. failed!");
    display.display();
    R = 2;
  }
  


  Serial.println("Ready!");



  delay(3000);

  display.clearDisplay();
  display.display();
  display.setTextSize(2);

}






void loop() {

  altimeter();

  writedisplay();

  buttons();

  sdwrite();
  


}


void altimeter() {
  // Get temp
  float tempc = (bmp.temperature);
  //conver C to F
  tempf = (tempc * 1.8) + 32;

  //get alt and convert to AGL
  float altagl = ((bmp.readAltitude(SEALEVELPRESSURE_HPA)) - altstart);
  //convert M to Ft
  altft = (altagl * 3.281);
}



void writedisplay() {
  Serial.println("writedisplay");
  Serial.println(R);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print ("Temp:");
  display.print (tempf, 1);
  display.println("F");
  display.print("AGL:");
  display.print(altft, 0);
  display.println("ft");
  if (rtc.getHours() < 10)
    display.print("0");
  display.print(rtc.getHours());
  display.print(':');
  if (rtc.getMinutes() < 10)
    display.print("0");
  display.print(rtc.getMinutes());
  display.print(':');
  if (rtc.getSeconds() < 10)
    display.print("0");
  display.println(rtc.getSeconds());
  if (R == 1) 
    display.println ("Recording");  //record indication
  if (R == 2) 
    display.println (" No Card");
  display.display();
}


void buttons() {
  //Serial.println("buttons");
  if (!digitalRead(BUTTON_B)) altstart = (bmp.readAltitude(SEALEVELPRESSURE_HPA));
  if (!digitalRead(BUTTON_A)) {
    if (R == 0) {
      R = 1;
      String filename = ("/");
      filename += rtc.getHours();
      filename += rtc.getMinutes();
      filename += rtc.getSeconds();
      filename += ".csv";
    
      logfile = SD.open(filename, FILE_WRITE);
      if ( ! logfile ) {
        display.print("Couldnt create ");
        display.println(filename);
        display.display();
        }
      Serial.print("Writing to ");
      Serial.println(filename);
      }

  }
  if (!digitalRead(BUTTON_C)) {
    R = 0;
    logfile.close();
    }
}



void sdwrite() {
  if (R == 1){
  Serial.println("writeSD");
  String RTCtime = ("");
  Serial.println("1");
  if (rtc.getHours() < 10)
    RTCtime += "0";
  RTCtime += rtc.getHours();
  Serial.println("2");
  RTCtime += ":";
  if (rtc.getMinutes() < 10)
    RTCtime += "0";
  RTCtime += rtc.getMinutes();
  Serial.println("3");
  RTCtime += ":";
  if (rtc.getSeconds() < 10)
    RTCtime += "0";
  RTCtime += rtc.getSeconds();
  Serial.println("4");
  digitalWrite(8, HIGH);
  Serial.println("5");
  logfile.print(RTCtime); logfile.print(" , "); logfile.print(tempf, 1); logfile.print(" , "); logfile.println(altft, 0);
  Serial.print(RTCtime); Serial.print(" "); Serial.print(altft, 0); Serial.println("ft");
  Serial.println("6");
  digitalWrite(8, LOW);
  Serial.println("7");
  logfile.flush();
  Serial.println("8");
  
  }
}
