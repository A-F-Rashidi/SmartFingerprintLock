// Libraries for display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Libraries for fingerprint
#include <HardwareSerial.h>
#include <Adafruit_Fingerprint.h>

// WiFi and mail
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h>


// WiFi credentials
const char* ssid = "Galaxy S24 Ultra E34D";
const char* password = "amir1234";

// Gmail SMTP server info
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "espfingirprintproject@gmail.com"     // Your Gmail address
#define AUTHOR_PASSWORD "mbeqkrpdoheizokz"                 // App Password from Google Account
#define RECIPIENT_EMAIL "espfingirprintproject@gmail.com"  // Recipient email (can be your own)

// OLED display config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Fingerprint sensor pins
#define RX_PIN 16
#define TX_PIN 17

#define BUTTON_PIN 27
#define BUZZER_PIN 15

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

SMTPSession smtp;
SMTP_Message message;

// Function to show messages on OLED
void showMessage(const String& line1, const String& line2 = "") {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(line1);
  if (line2 != "") display.println(line2);
  display.display();
}

// Callback for SMTP status
void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("Email sent successfully");
  }
}

// Send email function
void sendEmail() {
  smtp.callback(smtpCallback);

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);

  String messageBody = "The smart lock has just been opened by ID " + String(finger.fingerID) + "\n";
  messageBody += "Date&Time: ";
  messageBody += timeString;

  message.sender.name = "Smart Lock";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Smart Lock Opened";
  message.addRecipient("User", RECIPIENT_EMAIL);
  message.text.content = messageBody.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session)) {
    Serial.println("SMTP connection failed.");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Sending failed: " + smtp.errorReason());
  }

  smtp.closeSession();
}


// Read fingerprint and return status
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return p;

  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence ");
  Serial.println(finger.confidence);
  return FINGERPRINT_OK;
}

// Enroll fingerprint function
void enrollFingerprint() {
  int id = 0;
  while (finger.loadModel(id) == FINGERPRINT_OK) id++;
  if (id > 127) id = 0;

  showMessage("Enrolling ID:", String(id));
  delay(1000);

  showMessage("Place finger...");
  while (finger.getImage() != FINGERPRINT_OK)
    ;
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    showMessage("Failed at step 1");
    return;
  }

  showMessage("Remove finger");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER)
    ;

  showMessage("Place again...");
  while (finger.getImage() != FINGERPRINT_OK)
    ;
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    showMessage("Failed at step 2");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    showMessage("Model creation failed");
    return;
  }

  if (finger.storeModel(id) == FINGERPRINT_OK) {
    showMessage("Enroll success!", "ID: " + String(id));
  } else {
    showMessage("Enroll failed!");
  }

  delay(2000);
}

// Delete fingerprint by ID
void deleteFingerprint(int id) {
  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    showMessage("Deleted ID", String(id));
    Serial.println("Successfully deleted");
  } else {
    showMessage("Delete failed");
    Serial.println("Failed to delete");
  }
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1)
      ;
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  showMessage("Starting...", "Please wait");

  // Fingerprint sensor init
  mySerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  finger.begin(57600);
  delay(100);

  if (!finger.verifyPassword()) {
    showMessage("Sensor NOT found", "Check wiring!");
    while (1)
      ;
  }
  //Deleting All FingerPrints Every Time.
  if (finger.emptyDatabase() == FINGERPRINT_OK) {
    Serial.println("All fingerprints deleted.");
    showMessage("DB Cleared", "All fingerprints");
  } else {
    Serial.println("Failed to delete DB.");
    showMessage("DB Clear FAILED");
  }

  showMessage("Sensor detected");

  // WiFi connect
  WiFi.begin(ssid, password);
  showMessage("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // NTP time config
  configTime(3 * 3600 + 1800, 0, "pool.ntp.org", "time.nist.gov");  // UTC+3:30
  Serial.println("Waiting for time sync...");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized.");

  delay(1000);
  showMessage("Place your finger");
}

void loop() {
  static unsigned long lastClickTime = 0;
  static int clickCount = 0;
  static unsigned long pressStart = 0;

  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

  if (buttonPressed) {
    if (pressStart == 0) pressStart = millis();
    if (millis() - pressStart > 2000) {
      enrollFingerprint();
      clickCount = 0;
      pressStart = 0;
      showMessage("Place your finger");
      return;
    }
  } else {
    if (pressStart > 0 && millis() - pressStart < 500) {
      unsigned long now = millis();
      if (now - lastClickTime < 500) clickCount++;
      else clickCount = 1;
      lastClickTime = now;

      if (clickCount == 2) {
        showMessage("Enter ID to", "REMOVE");
        while (!Serial.available())
          ;
        int id = Serial.parseInt();
        deleteFingerprint(id);
        clickCount = 0;
        showMessage("Place your finger");
        return;
      }
    }
    pressStart = 0;
  }

  uint8_t result = getFingerprintID();
  switch (result) {
    case FINGERPRINT_OK:
      showMessage("Fingerprint matched", "Welcome! ID: " + String(finger.fingerID));
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);

      sendEmail();
      break;
    case FINGERPRINT_NOTFOUND:
      showMessage("Fingerprint", "Not recognized");
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      break;
    case FINGERPRINT_NOFINGER:
      break;
    default:
      showMessage("Read error", String("Code: ") + result);
      delay(2000);
      break;
  }

  delay(100);
}