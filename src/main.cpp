#include <Arduino.h>
#include "NeoPixelBus.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> leftShirt(18 * 2 + 50, 2); //
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> rightShirt(18 * 2 + 50, 6);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip3(45, 10);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip4(45, 14);

Adafruit_MPU6050 mpu;
TwoWire mpuI2C(0, 1);

extern void default_animation();
extern void setup_default_animation();

extern void nemo_light();
extern void setup_nemo_light();

int current_animation = 1;
void (*animations[])() = {default_animation, nemo_light};
void (*setup_animations[])() = {setup_default_animation, setup_nemo_light};

int hue;
int changeCooldown;
double lastMagAcc;
double lastMagGyro;
int directions[6] = {0, 0, 0, 0, 0,0};

int delay_time = 10;


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

    /*if (!mpu.begin(0x68, &mpuI2C, 0)) {
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
    }*/

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
    Serial.println("Connect MPU");
    delay(100);

    Serial.println("Starting");

    setup_animations[current_animation]();
}

void setup_default_animation() {
    hue = 0;
    changeCooldown = 0;
    lastMagAcc = 0;
    lastMagGyro = 0;
    delay_time = 10;
}

void default_animation() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    double magnitudeAcc = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y +
                               a.acceleration.z * a.acceleration.z);
    double magnitudeGyro = sqrt(g.gyro.x * g.gyro.x + g.gyro.y * g.gyro.y + g.gyro.z * g.gyro.z);

    double blendedMagAcc = magnitudeAcc + lastMagAcc / 2;
    lastMagAcc = blendedMagAcc * 0.7;
    double blendedMagGyro = magnitudeGyro + lastMagGyro / 2;
    lastMagGyro = blendedMagGyro * 0.7;

    Serial.print("Acc: ");
    Serial.println(blendedMagAcc);
    Serial.print("Gyro: ");
    Serial.println(blendedMagGyro);

    for (int i = 0; i < 50; i++) {
        HsbColor color = HsbColor(((i * 3 + hue) % 360) / 360.0, max(1 - (abs(blendedMagAcc - 10) / 30.0), 0), 0.25);
        leftShirt.SetPixelColor(i, color);
        rightShirt.SetPixelColor(i, color);
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
        }
    } else {
        RgbColor color1 = strip3.GetPixelColor(0);
        RgbColor newColor1 = color1.Dim(245);
        RgbColor color2 = strip4.GetPixelColor(0);
        RgbColor newColor2 = color2.Dim(245);
        for (int i = 0; i < 45; i++) {
            strip3.SetPixelColor(i, newColor1);
            strip4.SetPixelColor(i, newColor2);
        }
        changeCooldown = max(changeCooldown - 1, 0);
    }

    strip3.Show();
    strip4.Show();

    hue++;
    hue %= 360;
}

void setup_nemo_light() {
    changeCooldown = 0;
    delay_time = 50;
    for (int i = 0; i < 6; i++) {
        directions[i] = 0;
    }
}

int sgn(int a) {
    if (a > 0) {
        return 1;
    } else if (a < 0) {
        return -1;
    } else {
        return 0;
    }
}

void nemo_light() {
    leftShirt.ShiftRight(1);
    leftShirt.SetPixelColor(0, strip3.GetPixelColor(0));
    rightShirt.ShiftRight(1);
    rightShirt.SetPixelColor(0, strip4.GetPixelColor(0));

    strip3.ShiftLeft(1);
    strip4.ShiftLeft(1);

    for (int i = 0; i < 6; i++) {
        int add_direction = sqrt(random(0, 30));
        if (random(0, 2)) { // Sista exklusiv
            directions[i] += add_direction;
        } else {
            directions[i] -= add_direction;
        }
        directions[i] = max(min(directions[i], 50), -50);
    }

    HsbColor color1 = strip3.GetPixelColor(43);
    float newH = max(min((color1.H * 1000.0 + sgn(directions[0]) * 10) / 1000.0, 0.75), 0.5);
    float newS = max(min((color1.S * 1000.0 + sgn(directions[1])) / 1000.0, 0.8), 0.3);
    float newB = max(min((color1.S * 1000.0 + sgn(directions[2])) / 1000.0, 0.15), 0.05);

    strip3.SetPixelColor(44, HsbColor(newH, newS, newB));

    HsbColor color2 = strip4.GetPixelColor(43);

    newH = max(min((color2.H * 1000.0 + directions[3]) / 1000.0, 0.75), 0.5);
    newS = max(min((color2.S * 1000.0 + directions[4]) / 1000.0, 0.8), 0.3);
    newB = max(min((color2.S * 1000.0 + directions[5]) / 1000.0, 0.15), 0.05);

    strip4.SetPixelColor(44, HsbColor(newH, newS, newB));

    leftShirt.Show();
    rightShirt.Show();
    strip3.Show();
    strip4.Show();
}


unsigned long last_voltage_millis = millis();

void loop() {
    animations[current_animation](); // Gör om animationer så att de körs konstant i en loop i en funktion. Kolla ofta om animationen
    // behöver brytas eller om spänningen behöver kollas. (Istället för nuvarande metod)
    delay(delay_time);

    if (millis() - last_voltage_millis > 30000) {
        last_voltage_millis = millis();
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