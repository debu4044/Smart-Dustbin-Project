#include <ESP32Servo.h>

Servo MyServo;

#define TRIG_1 19   
#define TRIG_2 26   
#define ECHO_PIN 25  
#define SERVO_PIN1 17  

int alert = 1;
int userthreshold = 20;
int fillthreshold = 8;
bool fillAlertActive = false;  

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_1, OUTPUT);
    pinMode(TRIG_2, OUTPUT);
    pinMode(ECHO_PIN, INPUT_PULLUP);

    digitalWrite(TRIG_1, LOW);
    digitalWrite(TRIG_2, LOW);
    MyServo.attach(SERVO_PIN1);
    MyServo.write(0); 
}

long getDistance(int trigPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 40000); 
    if (duration == 0) return -1;  

    return duration * 0.034 / 2; 
}

void loop() {
    long distance2 = getDistance(TRIG_2);
    Serial.print("Sensor 2 (Fill Sensor): "); Serial.println(distance2);
    checkFillCondition(distance2, fillthreshold);

    if (!fillAlertActive) {
        long distance1 = getDistance(TRIG_1);
        Serial.print("Sensor 1 (User Sensor): "); Serial.println(distance1);
        checkUserCondition(distance1, userthreshold);
    } else {
        Serial.println("Lid opening disabled due to fill alert!");
    }

    delay(500);
}

void checkUserCondition(int distance, int threshold) {
    Serial.print("User Sensor Distance: "); Serial.println(distance);

    if (distance > 0 && distance < threshold) {
        openLid();
        while (true) {
            long newDistance = getDistance(TRIG_1);
            if (newDistance >= threshold || newDistance == -1) {
                closeLid();
                break;
            }
            delay(100);
        }
    }
}

void checkFillCondition(int distance, int threshold) {
    Serial.print("Fill Sensor Distance: "); Serial.println(distance);

    if (distance == 0) {
        Serial.println("ERROR: No echo received from Fill Sensor!");
    } else if (distance > 0 && distance < threshold) {
        if (!fillAlertActive) {
            Serial.println("⚠ Fill alert! Dustbin is almost full.");
            fillAlertActive = true;
        }
    } else {
        if (fillAlertActive) {
            Serial.println("Fill alert cleared. Dustbin has space.");
            fillAlertActive = false;
        }
    }
}

void openLid() {
    Serial.println("Opening lid...");
    MyServo.write(180);
    delay(500);
}

void closeLid() {
    Serial.println("Closing lid...");
    MyServo.write(0);
    delay(500);
}
