/*
 * UAV Propeller Damage Detection System
 * 
 * Low-cost sensor fusion system for pre-flight propeller damage detection
 * in unmanned aerial vehicles (UAVs).
 * 
 * Hardware:
 *   - Arduino (Uno/Nano)
 *   - MPU6050 accelerometer (vibration sensing)
 *   - Analog microphone module on A0 (acoustic energy)
 *   - IR optical sensor on Pin 2 (RPM via interrupt)
 *   - ESC + brushless motor with propeller on Pin 9
 * 
 * Test Protocol:
 *   - 5 PWM steps: 20, 25, 30, 35, 40
 *   - Each step held for 5 seconds
 *   - 10-second cooldown between runs
 *   - Trigger: rotate propeller manually >10 pulses to start
 * 
 * Output: Serial CSV stream — Saat, Test_Asamasi, PWM, RPM, Ses_Enerjisi, Titresim_G, Durum
 */

#include <Servo.h>
#include <Wire.h>
#include <MPU6050_tockn.h> 

Servo esc;
MPU6050 mpu6050(Wire);

volatile unsigned long turSayaci = 0;
unsigned long sonZaman = 0;
float rpm = 0;

int testKademeleri[] = {20, 25, 30, 35, 40,}; 
int toplamKademe = 5; 
int suankiKademe = 0;
unsigned long kademeBaslangicZamani = 0;
bool testAktif = false;
bool triggerBekleniyor = true;
const int kademeSuresi = 5000; 

void setup() {
  Serial.begin(9600);
  
  esc.attach(9, 1000, 2000); 
  esc.write(0); 
  delay(1000);

  Wire.begin();
  mpu6050.begin();
  
  Serial.println("MSG,Sensor Kalibre Ediliyor... Lutfen Kimildatmayin.");
  mpu6050.calcGyroOffsets(true); 
  
  Serial.println("CLEARDATA"); 
  Serial.println("LABEL,Saat,Test_Asamasi,PWM,RPM,Ses_Enerjisi,Titresim_G,Durum"); 

  pinMode(2, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(2), sayacArtir, CHANGE);
  
  turSayaci = 0; 
  
  Serial.println("MSG,SISTEM HAZIR. Baslatmak icin pervaneyi cevirin.");
}

void loop() {
  mpu6050.update(); 

  if (triggerBekleniyor) {
    esc.write(0);
    
    if (turSayaci > 10) { 
      Serial.println("MSG,TEST BASLIYOR! Geri sayim...");
      delay(2000); 
      
      triggerBekleniyor = false;
      testAktif = true;
      suankiKademe = 0;
      kademeBaslangicZamani = millis();
      turSayaci = 0;
      sonZaman = millis();
    }
  }
  
  else if (testAktif) {
    if (millis() - kademeBaslangicZamani > kademeSuresi) {
      suankiKademe++; 
      kademeBaslangicZamani = millis(); 
      
      if (suankiKademe >= toplamKademe) {
        testAktif = false;
        esc.write(0); 
        
        Serial.println("MSG,TEST BITTI. Soguma bekleniyor...");
        delay(10000); 
        
        turSayaci = 0; 
        triggerBekleniyor = true; 
        Serial.println("MSG,SISTEM TEKRAR HAZIR.");
        return;
      }
    }
    
    esc.write(testKademeleri[suankiKademe]);

    if (millis() - sonZaman >= 100) { 
      
      detachInterrupt(digitalPinToInterrupt(2)); 
      rpm = (turSayaci / 2.0) * (60000.0 / (millis() - sonZaman)); 
      turSayaci = 0;
      attachInterrupt(digitalPinToInterrupt(2), sayacArtir, CHANGE); 
      
      int sinyalMax = 0; 
      int sinyalMin = 1023;
      unsigned long okumaBaslangic = millis();
      
      while (millis() - okumaBaslangic < 20) {
        int okunan = analogRead(A0);
        if (okunan > sinyalMax) sinyalMax = okunan;
        if (okunan < sinyalMin) sinyalMin = okunan;
      }
      int sesEnerjisi = sinyalMax - sinyalMin; 
      
      float titretisimX = mpu6050.getAccX();
      float titretisimY = mpu6050.getAccY();
      float toplamG = sqrt((titretisimX * titretisimX) + (titretisimY * titretisimY));
      
      Serial.print("DATA,TIME,"); 
      Serial.print(suankiKademe + 1); 
      Serial.print(",");
      Serial.print(testKademeleri[suankiKademe]); 
      Serial.print(",");
      Serial.print(rpm);
      Serial.print(",");
      Serial.print(sesEnerjisi); 
      Serial.print(",");
      Serial.print(toplamG); 
      Serial.println(",CALISIYOR");
      
      sonZaman = millis();
    }
  }
}

void sayacArtir() {
  turSayaci++;
}
