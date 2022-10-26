// SD-card datalogger shield wit RTC
// ADS1115
// NTC50, 
// 10Kohm resistor
//
//  TX                         A0 - NTC100, 10Kohm resistor (Pulldown)
//  RX                         D0
//  D1 - RTC-ADS1115-SCL       D5 - SD-CLK
//  D2 - RTC-ADS1115-SCA       D6 - SD-MISO
//  D3                         D7 - SD-MOSI
//  D4 - LED, R220 resistor    D8 - SD-CS
// GND                        3V3 - ADS115-VDD
//  5V
//
// Get library: Adafruit_ADS1X15, RTClib
// Add Wemos to IDE

#include <Adafruit_ADS1X15.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <String.h>

Adafruit_ADS1115 ads;
RTC_DS1307 rtc;

String timeStamp;
bool useSD = true;
uint16_t counter = 0;
int16_t adc0, adc1, adc2, adc3;
float volts0, volts1, volts2, volts3, temperatur;
const char* filename = "data.csv";
File file;
char buf[20]; // buffer for DateTime.tostr

// Calibration - Add messured values from voltage refrence
bool useValuesFromRef = false;  // change to true, when messured
uint16_t valueFromRef = 1;
uint8_t messuredValueFromRef = 1;
uint8_t messuredValue = 0;

// 1 = show messured (uncalibrated values)
float calibratedVoltageFactor0 = 1;
float calibratedVoltageFactor1 = 1;
float calibratedVoltageFactor2 = 1;
float calibratedVoltageFactor3 = 1;

// equation = a(x-Xo) + b(y - yo)
// float factor = ( valueFromRef 5V - valueFromRef 2.5V ) + ( messuredValueFromRef 5V - messuredValueFromRef 2.5V )
void CalibrateSensors() {
  if ( useValuesFromRef == true ) {
    calibratedVoltageFactor0 = ( valueFromRef /* 5V */ - valueFromRef /* 2.5V */ ) + ( /* messuredValueFromRef */ /* 5V */ - messuredValueFromRef /* 2.5V */ );
    calibratedVoltageFactor1 = ( valueFromRef /* 5V */ - valueFromRef /* 2.5V */ ) + ( /* messuredValueFromRef */ /* 5V */ - messuredValueFromRef /* 2.5V */ );
    calibratedVoltageFactor2 = ( valueFromRef /* 5V */ - valueFromRef /* 2.5V */ ) + ( /* messuredValueFromRef */ /* 5V */ - messuredValueFromRef /* 2.5V */ );
    calibratedVoltageFactor3 = ( valueFromRef /* 5V */ - valueFromRef /* 2.5V */ ) + ( /* messuredValueFromRef */ /* 5V */ - messuredValueFromRef /* 2.5V */ );
  }
}

void GetDateTime() {
  DateTime time = rtc.now();
  timeStamp = time.year()+time.month()+time.day()+time.hour()+time.minute()+time.second();
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }
  
  Wire.begin();

  /*
   * SET Sample rate
   * RATE_ADS1115_8SPS (0x0000)   ///< 8 samples per second
   * RATE_ADS1115_16SPS (0x0020)  ///< 16 samples per second
   * RATE_ADS1115_32SPS (0x0040)  ///< 32 samples per second
   * RATE_ADS1115_64SPS (0x0060)  ///< 64 samples per second
   * RATE_ADS1115_128SPS (0x0080) ///< 128 samples per second (default)
   * RATE_ADS1115_250SPS (0x00A0) ///< 250 samples per second
   * RATE_ADS1115_475SPS (0x00C0) ///< 475 samples per second
   * RATE_ADS1115_860SPS (0x00E0) ///< 860 samples per second
   */
  //ads.setDataRate(RATE_ADS1115_128SPS);

  /*
   * The ADC input range (or gain) can be changed via the following
   * functions, but be careful never to exceed VDD +0.3V max, or to
   * exceed the upper and lower limits if you adjust the input range!
   * Setting these values incorrectly may destroy your ADC!
   *                                                                ADS1115
   *                                                                -------
   * ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 0.1875mV (default)
   * ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 0.125mV
   * ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 0.0625mV
   * ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.03125mV
   * ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.015625mV
   * ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
   */  
  Serial.println("Initialize ADS...");
  //ads.setGain(GAIN_TWOTHIRDS);
  if (!ads.begin()) { Serial.println(">>> Failed to initialize ADS."); while (1);}

  Serial.println("Initializing SD card...");
  if (!SD.begin()) { 
     Serial.println(">>> SD-Card failed, or not present"); 
     Serial.println(">>> Continiuing without SD-Card...");
     useSD = false;
   } else {
     Serial.println("card initialized.");
   }

  Serial.println("Initializing RTC...");
  if (! rtc.begin()) { Serial.println("Couldn't find RTC");while (1);}
  GetDateTime();
  Serial.println("Time set to: " + timeStamp + " adjusting...");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));     // Adjust at compile time, from computer time
  GetDateTime();
  Serial.println("Time updated to: " + timeStamp);

  Serial.println("Calibrate sensors...");
  pinMode(A0, INPUT);
  CalibrateSensors();

  pinMode(4, OUTPUT);

  Serial.println("Setup compleated...");
  Serial.println("Start messuring");
}

/*
 * Steinhart–Hart β parameter equation, 1/T = 1/To + 1/B In(R/Ro)
 * T – Temperature
 * To – Nominal Temperature, 25 °C or 298.15 K
 * B – Beta co-efficient
 * R – Measured resistance of the thermistor
 * Ro – Nominal Resistance, resistance at temperature T0 is 25 °C or 298.15K
 * The temperature value in kelvin for the respective resistance (R) of NTC thermistor, T = 1 / (1/To + 1/B In(R/Ro))
 * 
 * Both the module and circuit have a 50K NTC thermistor which has a nominal resistance of 50 Kohms at nominal temperature 
 * value T0 25 °C or 298.15K, also the series resistance is 10K in both circuits. So change the “Ro” value and Beta value in 
 * the code with the corresponding thermistor used in the circuit and “Rseries with series resistance value. 
 * In the code the resistance values are in kilo-ohms, so just use 10 or 50 for 10K or 50K values.
 */
float ComputeTemperatur() {
  uint16_t Ro = 100;      // Nominal resistance 100K, 
  uint16_t B =  3950;     // Beta constant from dataskeet
  uint16_t Rseries = 10;  // Series resistor 10K
  float To = 298.15;      // Nominal Temperature from dataskee

  //Read analog outputof NTC module, i.e the voltage across the thermistor
  float Vi = analogRead(A0) * (5.0 / 1023.0);
  
  //Convert voltage measured to resistance value, All Resistance are in kilo ohms.
  float R = (Vi * Rseries) / (5 - Vi);
  
  //Use R value in steinhart and hart equation Calculate temperature value in kelvin
  float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
  float Tc = T - 273.15; // Converting kelvin to celsius

  return Tc;
}

void UpdateReadings() {
  GetDateTime();
  counter++;
  
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);

  volts0 = ads.computeVolts(adc0) * calibratedVoltageFactor0;
  volts1 = ads.computeVolts(adc1) * calibratedVoltageFactor1;
  volts2 = ads.computeVolts(adc2) * calibratedVoltageFactor2;
  volts3 = ads.computeVolts(adc3) * calibratedVoltageFactor3;

  temperatur = ComputeTemperatur(); 
}

void WriteToSD() {
  if (useSD) {
    file = SD.open(filename, FILE_WRITE);
 
    if (file.size() == 0) {
    file.println("%d, %c, %d, C, %f, V, %f, V, %f, V, %f, V, counter, timeStamp, temperatur, volts0, volts1, volts2, volts3\0");
    file.flush();
    } 
  }
}

void ExportToSerial() {
    Serial.write("%d, %c, %d, C, %f, V, %f, V, %f, V, %f, V, counter, timeStamp, temperatur, volts0, volts1, volts2, volts3\0");
}

void loop() {
digitalWrite(4, HIGH);
UpdateReadings();
WriteToSD();
ExportToSerial();
digitalWrite(4, LOW);
delay(1000);
}
