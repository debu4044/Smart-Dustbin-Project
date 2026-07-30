#include <ESP32Servo.h>
#include <math.h>

#define SENS1THRES 16.0
#define DUSTBINRADIUS 20.0
#define DUSTHEIGHT 40.0
#define LEVELAREA 1256.0
#define RADARSTART 31
#define RADAREND 150
#define GROUP_SIZE 5   
#define RESET_BTN 26   
#define FILL_THRESHOLD 80.0  
typedef enum { MONITORING, PROCESSING, SHUTDOWN } dustbinstatus;
dustbinstatus mydustbin = MONITORING;
bool buttonstate = false;

class DistanceSensor {
  private:
    int trigpin, echopin;
  public:
    DistanceSensor(int trigpin, int echopin) {
      this->trigpin = trigpin;
      this->echopin = echopin;
      pinMode(this->trigpin, OUTPUT);
      pinMode(this->echopin, INPUT);
    }
    float getDistance() {
      digitalWrite(trigpin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigpin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigpin, LOW);
      long duration = pulseIn(echopin, HIGH, 30000);
      return (duration * 0.034 / 2);
    }
};

int ECHOPIN = 16;
int TRIGPINS[3] = {5, 18, 19};
DistanceSensor Sensor[] = { DistanceSensor(TRIGPINS[0], ECHOPIN), DistanceSensor(TRIGPINS[1], ECHOPIN), DistanceSensor(TRIGPINS[2], ECHOPIN) };

class ServoMotor {
  private:
    Servo myServo;
    int Servopin;
    int Index;
  public:
    ServoMotor(int servopin, int index) {
      Servopin = servopin;
      Index = index;
      myServo.attach(Servopin);
    }
    void openservo() {
      if (Index == 0) {
        for (int i = 0; i <= 90; i++) {
          myServo.write(i);
          delay(15);
        }
      }
    }
    void closeservo() {
      if (Index == 0) {
        for (int i = 90; i >= 0; i--) {
          myServo.write(i);
          delay(15);
        }
      }
    }
    void movetoangle(int rotangle) {
      myServo.write(rotangle);
      delay(20);
    }
};

ServoMotor Servo[] = {ServoMotor(23, 0), ServoMotor(25, 1)};

float averageGroupDistance(DistanceSensor &sensor, ServoMotor &servo, int startAngle, int endAngle) {
  float sum = 0;
  int count = 0;
  for (int angle = startAngle; angle <= endAngle; angle++) {
    servo.movetoangle(angle);
    float d = sensor.getDistance();
    if (d > 0) sum += d;
    count++;
    delay(15);
  }
  return (count > 0) ? (sum / count) : 0;
}

float sweepAndCalculate() {
  float totalUnoccupiedArea = 0;

  for (int groupStart = RADARSTART; groupStart <= RADAREND; groupStart += GROUP_SIZE) {
    int groupEnd = groupStart + GROUP_SIZE - 1;
    if (groupEnd > RADAREND) groupEnd = RADAREND;

    float avgDist = averageGroupDistance(Sensor[2], Servo[1], groupStart, groupEnd);

    // Area for this group segment
    float groupAngleRad = (groupEnd - groupStart + 1) * PI / 180.0;  // in radians
    float segmentArea = pow(avgDist, 2) * tan(groupAngleRad);

    totalUnoccupiedArea += segmentArea;
  }

  float wasteArea = LEVELAREA - totalUnoccupiedArea;
  if (wasteArea < 0) wasteArea = 0; 

  float fillPercent = (wasteArea / LEVELAREA) * 100.0;

  Serial.print("Waste Occupied Area: "); Serial.println(wasteArea);
  Serial.print("Fill Level: "); Serial.print(fillPercent); Serial.println("%");

  return fillPercent;
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Initialised");
  pinMode(RESET_BTN, INPUT_PULLUP); // button to reset shutdown
}

void loop() {
  float distance = Sensor[mydustbin].getDistance();

  switch (mydustbin) {
    case MONITORING:
      if (distance < SENS1THRES) {
        Serial.println("User detected — Opening...");
        Servo[0].openservo();
        while (Sensor[0].getDistance() <= SENS1THRES) {
          Servo[0].movetoangle(90);
        }
        Servo[0].closeservo();
        mydustbin = PROCESSING;
      }
      break;

    case PROCESSING: {
      if (distance < 10) {
        Serial.println("Dustbin full — Scanning...");
        if (Sensor[1].getDistance() < (0.2 * DUSTHEIGHT)) {
          float fillPercent = sweepAndCalculate();
          if (fillPercent >= FILL_THRESHOLD) {
            Serial.println("Fill level >= 80%. Entering SHUTDOWN!");
            mydustbin = SHUTDOWN;
          } else {
            mydustbin = MONITORING;
          }
        } 
      }else {
          mydustbin = MONITORING;
        }
    } break;

    case SHUTDOWN:
      Serial.println("Dustbin in SHUTDOWN. Press button to reset.");
      Serial.println("The button is ready to accept the input");
      delay(1000);
      buttonstate = digitalRead(RESET_BTN);
      if (buttonstate == LOW) {  
        Serial.println("Button pressed — Returning to monitoring");
        mydustbin = MONITORING;
      }else{
        mydustbin = PROCESSING;
      }
      break;
  }

  delay(200);
}
