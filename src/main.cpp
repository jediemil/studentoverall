#include <Arduino.h>
#include "NeoPixelBus.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> leftShirt(18*2 + 50, 2); //
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> rightShirt(18 * 2 + 50, 6);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip3(45, 10);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip4(45, 14);

Adafruit_MPU6050 mpu;
TwoWire mpuI2C(0, 1);


void setup() {
    Serial.begin(115200);

    delay(1000);

    Serial.println("Booted");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    analogReadResolution(12);

    leftShirt.Begin();
    rightShirt.Begin();
    strip3.Begin();
    strip4.Begin();

    leftShirt.ClearTo(RgbColor(0, 0, 0));
    rightShirt.ClearTo(RgbColor(0, 0, 0));
    strip3.ClearTo(RgbColor(0, 0, 0));
    strip4.ClearTo(RgbColor(0, 0, 0));
    leftShirt.Show();
    rightShirt.Show();
    strip3.Show();
    strip4.Show();

    if (!mpu.begin(0x68, &mpuI2C, 0)) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) {
            delay(100);
            leftShirt.ClearTo(RgbColor(80, 0, 0));
            rightShirt.ClearTo(RgbColor(80, 0, 0));
            strip3.ClearTo(RgbColor(80, 0, 0));
            strip4.ClearTo(RgbColor(80, 0, 0));
            leftShirt.Show();
            rightShirt.Show();
            strip3.Show();
            strip4.Show();
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
    Serial.println("Connect MPU");
    delay(100);

    Serial.println("Starting");
}


int hue = 0;
int changeCooldown = 0;
double lastMagAcc = 0;
double lastMagGyro = 0;

void loop() {
    /*Serial.println("Working");
    for (int a = 0; a < 360; a++) {
        for (int i = 0; i < 100; i++) {
            strip1.SetPixelColor(i, HsbColor(((a + i) % 360) / 360.0, 1, 0.25));
            rightShirt.SetPixelColor(i, RgbColor(255 * a / 360.0, 0, 0));
            strip3.SetPixelColor(i, RgbColor(0, 255, 100));
            strip4.SetPixelColor(i, HsbColor((1 - ((a + i) % 360) / 360.0), 1, 0.25));
        }
        strip1.Show();
        rightShirt.Show();
        strip3.Show();
        strip4.Show();
        delay(20);
    }*/

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    /*Serial.print("AccelX:");
    Serial.print(a.acceleration.x);
    Serial.print(",");
    Serial.print("AccelY:");
    Serial.print(a.acceleration.y);
    Serial.print(",");
    Serial.print("AccelZ:");
    Serial.print(a.acceleration.z);
    Serial.print(", ");
    Serial.print("GyroX:");
    Serial.print(g.gyro.x);
    Serial.print(",");
    Serial.print("GyroY:");
    Serial.print(g.gyro.y);
    Serial.print(",");
    Serial.print("GyroZ:");
    Serial.print(g.gyro.z);
    Serial.println("");*/

    double magnitudeAcc = sqrt(a.acceleration.x*a.acceleration.x + a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z);
    double magnitudeGyro = sqrt(g.gyro.x*g.gyro.x + g.gyro.y*g.gyro.y + g.gyro.z*g.gyro.z);

    /*Serial.print("Acc: ");
    Serial.println(magnitudeAcc);
    Serial.print("Gyro: ");
    Serial.println(magnitudeGyro);*/

    double blendedMagAcc = magnitudeAcc + lastMagAcc/2;
    lastMagAcc = blendedMagAcc * 0.7;
    double blendedMagGyro = magnitudeGyro + lastMagGyro/2;
    lastMagGyro = blendedMagGyro * 0.7;

    Serial.print("Acc: ");
    Serial.println(blendedMagAcc);
    Serial.print("Gyro: ");
    Serial.println(blendedMagGyro);

    for (int i = 0; i < 50; i++) {
        HsbColor color = HsbColor(((i*3 + hue) % 360) / 360.0, max(1 - (abs(blendedMagAcc - 10) / 30.0), 0), 0.25);
        leftShirt.SetPixelColor(i, color);
        rightShirt.SetPixelColor(i, color);
        //strip3.SetPixelColor(i, RgbColor(0, 255, 100));
        //strip4.SetPixelColor(i, HsbColor((1 - ((a + i) % 360) / 360.0), 1, 0.25));
    }
    for (int i = 0; i < 18; i++) {
        HsbColor color = RgbColor(0, max(100 - (blendedMagAcc * 4 + blendedMagGyro * 4), 0), 0);
        HsbColor color2 = RgbColor(0, 0, max(100 - (blendedMagAcc * 4 + blendedMagGyro * 4), 0));
        leftShirt.SetPixelColor(i + 50, color);
        rightShirt.SetPixelColor(i + 50, color);
        leftShirt.SetPixelColor(i + 50 + 18, color2);
        rightShirt.SetPixelColor(i + 50 + 18, color2);
    }

    leftShirt.Show();
    rightShirt.Show();

    if (magnitudeAcc * 4 + magnitudeGyro * 4 > 80 && changeCooldown == 0) {
        changeCooldown = 10;
        HsbColor color1 = HsbColor(random(0, 1000) / 1000.0, 1, 0.25);
        HsbColor color2 = HsbColor(random(0, 1000) / 1000.0, 1, 0.25);
        for (int i = 0; i < 45; i++) {
            strip3.SetPixelColor(i, color1);
            strip4.SetPixelColor(i, color2);
            //strip3.SetPixelColor(i, RgbColor(0, 255, 100));
            //strip4.SetPixelColor(i, HsbColor((1 - ((a + i) % 360) / 360.0), 1, 0.25));
        }
    } else {
        RgbColor color1 = strip3.GetPixelColor(0);
        RgbColor newColor1 = color1.Dim(245);
        RgbColor color2 = strip4.GetPixelColor(0);
        RgbColor newColor2 = color2.Dim(245);
        for (int i = 0; i < 45; i++) {
            strip3.SetPixelColor(i, newColor1);
            strip4.SetPixelColor(i, newColor2);
            //strip3.SetPixelColor(i, RgbColor(0, 255, 100));
            //strip4.SetPixelColor(i, HsbColor((1 - ((a + i) % 360) / 360.0), 1, 0.25));
        }
        changeCooldown = max(changeCooldown - 1, 0);
    }

    strip3.Show();
    strip4.Show();

    delay(10);
    hue++;
    hue %= 360;

    if (hue == 0) {
        unsigned long adcReadings = 0;
        analogRead(26);
        for (int i = 0; i < 20; i++) {
            adcReadings += analogRead(26);
            delay(1);
        }
        double voltage = (adcReadings / 4095.0 / 20.0) * 3.3;
        double batVoltage = voltage * 5.7 + 0.2; //Sus + 0.4?
        Serial.print("Bat voltage: ");
        Serial.println(batVoltage);

        /*rightShirt.SetPixelColor(round(batVoltage*2), RgbColor(255, 255, 255));
        rightShirt.Show();
        while (1) {
            delay(1000);
        }*/

        if (batVoltage < 14) {
            leftShirt.ClearTo(RgbColor(0, 0, 0));
            rightShirt.ClearTo(RgbColor(50, 0, 0));
            strip3.ClearTo(RgbColor(0, 0, 0));
            strip4.ClearTo(RgbColor(0, 0, 0));
            leftShirt.Show();
            rightShirt.Show();
            strip3.Show();
            strip4.Show();

            while (1) {
                delay(1000);
            }
        }
    }
}