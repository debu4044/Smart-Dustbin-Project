#include <ESP32Servo.h>
#include <math.h>

const int echo = 25;
const int trig1 = 21;
const int trig2 = 26;
const int trig3 = 33;
float distancerec1, distancerec2, distancerec3;
int fillalert = 0;
const int servopin1 = 19;
const int servopin2 = 18;
const int buzzerpin = 4;
const float dustbinht = 80;  // Adjust according to dustbin size
const float longitudinalthres = 72;  // Threshold for fill level
int shiftcontrol = 0;
int obsdetectionrange = 20;
bool lidOpened = false;
int arr[180];
float angle;
const float radius = 20;
int count = 0;

class Sensor {
private:
    int echopin;
    int trigpin;
public:
    Sensor(int trig) {
        trigpin = trig;
        echopin = echo;
        pinMode(trigpin, OUTPUT);
        pinMode(echopin, INPUT);
    }
    float getDistance() {
        digitalWrite(trigpin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigpin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigpin, LOW);
        long duration = pulseIn(echopin, HIGH, 30000);
        float distanceCM = duration * 0.034 / 2;
        return distanceCM;
    }
};

Servo MyServo[2];
Sensor MySensor1(trig1);
Sensor MySensor2(trig2);
Sensor MySensor3(trig3);

void setup() {
    Serial.begin(115200);
    Serial.println("Hello, ESP32!");

    MyServo[0].attach(servopin1);
    MyServo[1].attach(servopin2);
    pinMode(buzzerpin, OUTPUT);

    for (int pos = 30; pos <= 150; pos++) {
        angle = 3.14 * pos / 180;
        arr[pos] = 2 * radius * sin(angle);
    }
}

void loop() {
    distancerec2 = MySensor2.getDistance();
    Serial.print("Sensor 2 readings: ");
    Serial.println(distancerec2);
    delay(5);
    if (distancerec2 < (dustbinht - longitudinalthres)) {
        shiftcontrol = 1;
    }

    if (shiftcontrol == 1) {
        count = 0;
        for (int pos = 30; pos <= 150; pos++) {
            MyServo[1].write(pos);
            distancerec3 = MySensor3.getDistance();
            Serial.print("Sensor 3 readings at ");
            Serial.print(pos);
            Serial.print(" degrees: ");
            Serial.println(distancerec3);
            delay(15);
            
            if (distancerec3 < arr[pos]) {
                count++;
            }
        }

        float fillPercentage = (float)count / 121.0 * 100.0;
        if (fillPercentage > 50) {
            Serial.println("Fill level is more than 50%! Fill Alert!!!");
            Serial.println("Sensor lid opening is suspended temporarily, u can now open in manually only!!!!");
            fillalert = 1;
            digitalWrite(buzzerpin, HIGH);
            delay(3000);
            digitalWrite(buzzerpin, LOW);
        }
        shiftcontrol = 0;
    }

    if (fillalert == 0) {
        distancerec1 = MySensor1.getDistance();
        Serial.print("Sensor 1 readings: ");
        Serial.println(distancerec1);
        delay(5);

        if (distancerec1 < obsdetectionrange) {
            openLidSmoothly();
            lidOpened = true;
        }
        unsigned long startTime = millis();
        while (distancerec1 < obsdetectionrange) {
            Serial.println("Lid is open!!!!");
            distancerec1 = MySensor1.getDistance();

            if (millis() - startTime > 10000) { 
                Serial.println("Sensor stuck, exiting loop.");
                break;
            }
        }
        if (lidOpened) {
            closeLidSmoothly();
            lidOpened = false;
        }
    }
    fillalert = 0;
    delay(10);
}
void openLidSmoothly() {
    Serial.println("Lid is opening...");
    for (int pos = 0; pos <= 180; pos++) {
        MyServo[0].write(pos);
        delay(5);
    }
}
void closeLidSmoothly() {
    Serial.println("Lid is closing...");
    for (int pos = 180; pos >= 0; pos--) {
        MyServo[0].write(pos);
        delay(5);
    }
}
