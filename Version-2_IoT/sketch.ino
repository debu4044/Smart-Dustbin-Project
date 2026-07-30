#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Automated Waste Manager"

#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <ThingSpeak.h>
#include <base64.h>

#define BLYNK_PRINT Serial

// Wi-Fi & Blynk Credentials
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// Twilio Settings
const char* twilioHost = "api.twilio.com";
const int twilioPort = 443;

const char* accountSID = "YOUR_TWILIO_ACCOUNT_SID";
const char* authToken = "YOUR_TWILIO_AUTH_TOKEN";
const char* twilioNumber = "+1XXXXXXXXXX";      // Your Twilio number
const char* recipientNumber = "+91XXXXXXXXXX";  // Recipient phone number
bool notificationSent = false;                       // Flag to prevent repeat notifications

Servo MyServo;
#define TRIG_1 D8   
#define TRIG_2 D2   
#define ECHO_PIN D7
#define Buzzerpin D5  
int SERVO_PIN = D6;
int userthreshold = 20;
int fillthreshold = 8;
bool lidOpen = false;
bool fillAlert = false;
bool prevFillAlert = false;  // Track previous state to detect changes
unsigned long lastThingSpeakTime = 0;
WiFiClient client;

// ThingSpeak channel details
unsigned long myChannelNumber = 2884544;
const char *myWriteAPIKey = "52GGKW3B87JOVV5Q";

void setup() {
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);
    ThingSpeak.begin(client); // Init ThingSpeak
    pinMode(TRIG_1, OUTPUT);
    pinMode(TRIG_2, OUTPUT);
    pinMode(ECHO_PIN, INPUT_PULLUP);
    digitalWrite(TRIG_1, LOW);
    digitalWrite(TRIG_2, LOW);
    
    MyServo.attach(SERVO_PIN);
    MyServo.write(0);
    
    Serial.println("Automated Waste Manager with Twilio notifications initialized");
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

void openLid() {
    if (!lidOpen) {
        Serial.println("Opening lid...");
        MyServo.write(180);
        lidOpen = true;
        Blynk.virtualWrite(V2, 1);
    }
}

void closeLid() {
    if (lidOpen) {
        Serial.println("Closing lid...");
        MyServo.write(5);  // Safer angle than -10 to avoid jitter
        lidOpen = false;
        Blynk.virtualWrite(V2, 0);
    }
}

// Function to send SMS via Twilio
bool sendTwilioSMS(String message) {
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate validation
    
    Serial.println("Connecting to Twilio...");
    if (!client.connect(twilioHost, twilioPort)) {
        Serial.println("Connection to Twilio failed");
        return false;
    }

    // Prepare the authentication header
    String auth_header = "Basic " + base64::encode(String(accountSID) + ":" + String(authToken));
    
    // Prepare the POST data
    String post_data = "To=" + urlEncode(String(recipientNumber)) + 
                      "&From=" + urlEncode(String(twilioNumber)) + 
                      "&Body=" + urlEncode(message);
    
    // Prepare the HTTP request
    String request = "POST /2010-04-01/Accounts/" + String(accountSID) + "/Messages.json HTTP/1.1\r\n" +
                    "Host: " + twilioHost + "\r\n" +
                    "Authorization: " + auth_header + "\r\n" +
                    "Content-Type: application/x-www-form-urlencoded\r\n" +
                    "Content-Length: " + String(post_data.length()) + "\r\n" +
                    "Connection: close\r\n\r\n" +
                    post_data;
    
    client.print(request);
    Serial.println("SMS request sent to Twilio");
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 10000) {
        if (client.available()) {
            String response = client.readStringUntil('\r');
            Serial.println(response);
            if (response.indexOf("HTTP/1.1 201") >= 0) {
                Serial.println("SMS sent successfully!");
                return true;
            }
        }
        delay(50);
    }
    
    Serial.println("Failed to send SMS or no response");
    return false;
}

// URL encode function for Twilio message parameters
String urlEncode(String str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
}

void loop() {
    Blynk.run();
    long distance1 = getDistance(TRIG_1);
    long distance2 = getDistance(TRIG_2);
    Serial.print("User Sensor: "); Serial.println(distance1);
    Serial.print("Fill Sensor: "); Serial.println(distance2);
    Blynk.virtualWrite(V0, distance1);
    Blynk.virtualWrite(V1, distance2);
    
    // Store previous fill state to detect changes
    prevFillAlert = fillAlert;
    
    // Bin Full Logic
    if (distance2 > 0 && distance2 < fillthreshold) {
        fillAlert = true;
        Serial.println("⚠ Bin Full! Auto-lid disabled.");
        Blynk.virtualWrite(V3, 255); // Red LED ON
        
        // Send notification when the bin first becomes full
        if (!prevFillAlert && !notificationSent) {
            Serial.println("Sending Twilio notification...");
            bool smsSent = sendTwilioSMS("Alert: Your waste bin is full and needs emptying!");
            if (smsSent) {
                notificationSent = true;
                Blynk.virtualWrite(V4, "Notification sent: Bin is full");
            }
        }
    } else {
        fillAlert = false;
        Blynk.virtualWrite(V3, 0);   // LED OFF
        // Reset notification flag when bin is emptied
        if (prevFillAlert && !fillAlert) {
            notificationSent = false;
            Blynk.virtualWrite(V4, "Bin level normal");
        }
    }
    
    // Auto lid logic
    if (!fillAlert) {
        if (distance1 > 0 && distance1 < userthreshold) {
            openLid();
        } else {
            closeLid();
        }
    }
    
    // Send to ThingSpeak every 15 seconds
    if (millis() - lastThingSpeakTime >= 15000) {
        ThingSpeak.setField(1, distance1);
        ThingSpeak.setField(2, distance2);
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (x == 200) {
            Serial.println("✅ ThingSpeak update successful.");
        } else {
            Serial.println("❌ ThingSpeak update failed. HTTP error: " + String(x));
        }
        lastThingSpeakTime = millis();
    }
    
    delay(500);
}

// Manual Blynk control
BLYNK_WRITE(V2) {
    int pinValue = param.asInt();
    if (pinValue == 1) {
        openLid();
        delay(2000);
    } else {
        closeLid();
        delay(1000);
    }
}

// Reset notification flag manually if needed
BLYNK_WRITE(V5) {
    int pinValue = param.asInt();
    if (pinValue == 1) {
        notificationSent = false;
        Serial.println("Notification flag reset manually");
        Blynk.virtualWrite(V5, 0); // Reset button state
    }
}
