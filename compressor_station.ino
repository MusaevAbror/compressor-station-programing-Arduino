#include <TroykaMQ.h>
#include <GParser.h>
#include <AsyncStream.h>
#include <microDS18B20.h>

#define pin_mq6 A5
#define pin_vib1 7
#define pin_vib2 8
#define pin_pojar 4
#define pin_M1 9
#define pin_M2 10
#define pin_curer 5




uint8_t addres_M1[] = {0x28, 0xFF, 0x64, 0x1, 0xBF, 0x63, 0x36, 0x46};
uint8_t addres_M2[] = {0x28, 0xFF, 0x64, 0x1, 0xBF, 0x7C, 0xA8, 0xBC};
uint8_t addres_T1[] = {0x28, 0xFF, 0x64, 0x1, 0xBC, 0x69, 0xE6, 0x12};
uint8_t addres_T2[] = {0x28, 0xFF, 0x64, 0x1, 0xBF, 0x28, 0xBB, 0x8F};



int gaz_ppm = 0;
boolean vib1, vib2, pojar;

volatile unsigned long lastM1_flash, lastM1_show, flash_M1;
volatile unsigned long flash_M2, lastM2_show, lastM2_flash;
unsigned long tempTime;

unsigned int RPM1, RPM2; // oborod m1, m2 matorni
float time_m1, time_m2; // oborod vaqti m1, m2, matorni
float temp_M1, temp_M2, temp_T1, temp_T2;

AsyncStream<50> serial(&Serial, ';');
MQ6 mq6(pin_mq6);
MicroDS18B20<A4, addres_M1> temM1;
MicroDS18B20<A4, addres_M2> temM2;
MicroDS18B20<A4, addres_T1> temT1;
MicroDS18B20<A4, addres_T2> temT2;



void setup() {
  TCCR1A = (TCCR1A & 0xF8) | 1;
  TCCR1B = (TCCR1B & 0xF8) | 1;
  
  Serial.begin(9600);
  pinMode(pin_vib1, INPUT);
  pinMode(pin_vib2, INPUT);
  pinMode(pin_pojar, INPUT);
  pinMode(pin_M1, OUTPUT);
  pinMode(pin_M2, OUTPUT);
  pinMode(pin_curer, OUTPUT);

  attachInterrupt(0, oborod_M1, RISING);
  attachInterrupt(1, oborod_M2, RISING);
  mq6.calibrate(8.2);
  Serial.setTimeout(5);
}


void loop() {
  //  serial_info();
  datchik_gaz();
  datchik_vib1_vib_2();
  datchik_pojar();
  TemperaturaInfo();
  oborodInfo();
  SerialSend();


}

void oborodInfo()
{
  if (micros() - lastM1_flash > 1000000)
  {
    RPM1 = 0;
  }
  else
  {
    time_m1 = (float)flash_M1 / 1000000;
    RPM1 = (float)60 / time_m1;
  }

  if (micros() - lastM2_flash > 1000000)
  {
    RPM2 = 0;
  }
  else
  {
    time_m2 = (float)flash_M2 / 1000000;
    RPM2 = (float)60 / time_m2;
  }

}
void oborod_M1()
{
  flash_M1 = micros() - lastM1_flash;
  lastM1_flash = micros();

}
void oborod_M2()
{
  flash_M2 = micros() - lastM2_flash;
  lastM2_flash = micros();

}
void datchik_gaz()
{
  gaz_ppm = mq6.readLPG();

}
void datchik_vib1_vib_2()
{
  vib1 = digitalRead(pin_vib1);
  vib2 = digitalRead(pin_vib2);
}
void datchik_pojar()
{
  pojar = !digitalRead(4);

}

//void serial_info()
//{
//  // index 1 t1 tranzistor D5 pin
//  // index 2 t2 tranzistor D6 pin
//  // index 3 curer  D9 pin
//
//
//}

void TemperaturaInfo()
{
  if (millis() - tempTime > 1300)
  {
    if (temM1.readTemp())
    {
      temp_M1 = temM1.getTemp();
    }
    temM1.requestTemp();

    if (temM2.readTemp())
    {
      temp_M2 = temM2.getTemp();
    }
    temM2.requestTemp();

    if (temT1.readTemp())
    {
      temp_T1 = temT1.getTemp();
    }
    temT1.requestTemp();

    if (temT2.readTemp())
    {
      temp_T2 = temT2.getTemp();
    }
    temT2.requestTemp();

    tempTime = millis();
  }
}
void SerialSend()
{
  Serial.print(temp_M1);
  Serial.print(',');
  Serial.print(temp_M2);
  Serial.print(',');
  Serial.print(temp_T1);
  Serial.print(',');
  Serial.print(temp_T2);
  Serial.print(',');
  Serial.print(RPM1);
  Serial.print(',');
  Serial.print(RPM2);
  Serial.print(',');
  Serial.print(gaz_ppm);
  Serial.print(',');
  Serial.print(vib1);
  Serial.print(',');
  Serial.print(vib2);
  Serial.print(',');
  Serial.print(pojar);
  Serial.println();

}
void yield()
{
    if (serial.available())
  {
    GParser data(serial.buf, ',');
    int ints[10];
    data.parseInts(ints);

    switch (ints[0])
    {
      case 1:

        analogWrite(pin_M1, ints[1]);

        break;
      case 2:

        analogWrite(pin_M2, ints[1]);

        break;
      case 3:
        analogWrite(pin_curer, ints[1]);
        break;
    }
  }
}
