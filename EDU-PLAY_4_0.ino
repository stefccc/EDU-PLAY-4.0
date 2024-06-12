//In order to compile and run this code, you need the install the additional libraries that are included

#include <Arduino.h>            // Core library for Arduino
#include <Adafruit_NeoPixel.h>  // Library for controlling NeoPixel LEDs
#include <Wire.h>               // Library for I2C communication
#include <hd44780.h>            // Library for HD44780 LCD display
#include <hd44780ioClass/hd44780_I2Cexp.h> // I2C expander class for HD44780
#include <Servo.h>              // Library for controlling servo motors
#ifdef __AVR__
#include <avr/power.h>          // Library for power management on AVR microcontrollers
#endif
#include <Stepper.h>            // Library for controlling stepper motors

// Custom heart icon for the LCD display
byte heartIcon[8] = {
    B01010,
    B11111,
    B11111,
    B01110,
    B01110,
    B00100,
    B00100,
    B00000
};

// Private variables
int life = 3, level = 1; // Initial lives and level
bool saveLife = false, minusLife = false, opened = false, firstCheck = true, restart = true, roundStart = false, command = true, pocket = false;

// Servo motor object
Servo myservo;

// Stepper motors objects
Stepper motorL = Stepper(150, 22, 23, 24, 25); // Left stepper motor
Stepper motorR = Stepper(150, 26, 27, 28, 29); // Right stepper motor

// Display object
hd44780_I2Cexp lcd;

// LED rings objects
// Note: Initially attempted to store LED rings in an array of objects for better structure.
// However, this approach caused functionality issues. Therefore, LED rings are managed separately.
Adafruit_NeoPixel ring0(12, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring1(12, 7, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2(12, 8, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring3(12, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring4(12, 10, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring5(12, 11, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring6(12, 12, NEO_GRB + NEO_KHZ800);

void setup() {
    // Setup rings for start
    setupRings();
    clearRings();
    updateRings();

    // Start serial communication -> used for debugging with a connected PC
    Serial.begin(9600);

    // Initialize LCD display
    lcd.begin(20, 4);
    lcd.backlight();

    // Set pins as INPUT
    for(int i = 30; i <= 43; i++){
        pinMode(i, INPUT);
    }

    // Set speed to motors
    motorL.setSpeed(150);
    motorR.setSpeed(150);

    // Initialize servo and set the default position
    myservo.attach(13);
    myservo.write(130);

    // Set pins as OUTPUT for controlling motors
    for(int i = 2; i <= 5; i++){
        pinMode(i, OUTPUT);
    }

    // Initial setup for the first LED ring and update
    colorWipe0(ring0.Color(0, 255, 0));
    updateRings();
    clearRings();
    lcd.createChar(1, heartIcon); // Create custom character on LCD
}

void loop() {
    if (Restart()) {
        if (StickDown()) {
            if (CheckPocket()) {
                Open();
                GameStart();
            } else {
                GameStart();
            }
        }
        MotorMove();
    } else {
        if (CheckPocket()) {
            if (!SaveLife() && minusLife) {
                MinusLife();
            }
            if (StickDown() && !opened) {
                Open();
            } else if (!StickDown()) {
                MotorMove();
            }
        } else {
            if (opened) {
                Close();
            }
            if (RoundStart()) {
                SetLevel();
            } else {
                if (CheckSensor()) {
                    Score();
                } else {
                    MotorMove();
                }
            }
        }
    }
}

// Function to initialize all LED rings
void setupRings() {
    ring0.begin();
    ring1.begin();
    ring2.begin();
    ring3.begin();
    ring4.begin();
    ring5.begin();
    ring6.begin();
    ring0.setBrightness(100);
    ring1.setBrightness(100);
    ring2.setBrightness(100);
    ring3.setBrightness(100);
    ring4.setBrightness(100);
    ring5.setBrightness(100);
    ring6.setBrightness(100);
}

// Function to clear all LED rings
void clearRings() {
    colorWipe0(ring0.Color(255, 0, 0));
    ring1.clear();
    ring2.clear();
    ring3.clear();
    ring4.clear();
    ring5.clear();
    ring6.clear();
}

// Function to update all LED rings
void updateRings() {
    ring0.show();
    ring1.show();
    ring2.show();
    ring3.show();
    ring4.show();
    ring5.show();
    ring6.show();
}

// Function to decrease lives and handle game over condition
void MinusLife() {
    LoseSound();
    if (--life <= 0) {
        lcd.setCursor(0, 2);
        lcd.print(" PRESS START BUTTON ");
        restart = true;
    }
    minusLife = false;
    DisplayUpdate();
}

// Function to check if the restart button is pressed
bool Restart() {
    if (digitalRead(37) == 0) {
        lcd.setCursor(0, 2);
        lcd.print("   GAME RESTARTED   ");
        restart = true;
        command = true;
        pocket = true;
    }
    return restart;
}

// Function to check if the stick is down
bool StickDown() {
    if (digitalRead(38) == HIGH && digitalRead(39) == HIGH) {
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        return true;
    } else {
        if (command) {
            lcd.setCursor(0, 3);
            lcd.print("BRING THE STICK DOWN");
            command = false;
        }
        return false;
    }
}

// Function to check if the ball is in the pocket (first sensor triggered)
bool CheckPocket() {
    if (digitalRead(30) == HIGH) {
        pocket = true;
    }
    if (pocket) {
        if (firstCheck) {
            colorWipe0(ring0.Color(0, 255, 0));
            firstCheck = false;
            command = true;
        }
        return true;
    }
    return false;
}

// Function to start the game (reset level and lives)
void GameStart() {
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    restart = false;
    roundStart = true;
    minusLife = false;
    life = 3;
    level = 1;
    DisplayUpdate();
    ring1.clear();
    ring1.show();
    ring2.clear();
    ring2.show();
    ring3.clear();
    ring3.show();
    ring4.clear();
    ring4.show();
    ring5.clear();
    ring5.show();
    ring6.clear();
    ring6.show();
}

// Function to return if player has scored recently
bool SaveLife() {
    bool temp = saveLife;
    return temp;
}

// Function to return if the round just started
bool RoundStart() {
    if (roundStart) {
        firstCheck = true;
        minusLife = true;
        saveLife = false;
        return true;
    }
    return false;
}

// Function to set the level by lighting up the appropriate LED ring
void SetLevel() {
    Serial.print("level is");
    Serial.println(level);
    switch(level){
        case 1:
            colorWipe1(ring1.Color(25, 255, 255));
            ring1.show();
            Serial.println("First level set");
            break;
        case 2:
            ring1.clear();
            ring1.show();
            colorWipe2(ring2.Color(25, 255, 255));
            ring2.show();
            Serial.println("Second level set");
            break;
        case 3:
            ring2.clear();
            ring2.show();
            colorWipe3(ring3.Color(25, 255, 255));
            ring3.show();
            break;
        case 4:
            ring3.clear();
            ring3.show();
            colorWipe4(ring4.Color(25, 255, 255));
            ring4.show();
            break;
        case 5:
            ring4.clear();
            ring4.show();
            colorWipe5(ring5.Color(25, 255, 255));
            ring5.show();
            break;
        case 6:
            ring5.clear();
            ring5.show();
            colorWipe6(ring6.Color(25, 255, 255));
            ring6.show();
            break;
        default:
            break;
    }
    DisplayUpdate();
    roundStart = false;
}

// Function to check if the player hit the right hole (sensor)
bool CheckSensor() {
    if (digitalRead(30 + level) == HIGH) {
        return true;
    }
    return false;
}

// Function to handle scoring
void Score() {
    SaveSound();
    level++;
    saveLife = true;
    command = true;
    pocket = false;
    firstCheck = true;
    ring1.clear();
    ring1.show();
    ring2.clear();
    ring2.show();
    ring3.clear();
    ring3.show();
    ring4.clear();
    ring4.show();
    ring5.clear();
    ring5.show();
    ring6.clear();
    ring6.show();
}

// Function to open the mechanism
void Open() {
    myservo.write(0);
    lcd.setCursor(0, 3);
    lcd.print("    DOOR OPENING    ");
    opened = true;
}

// Function to close the mechanism
void Close() {
    myservo.write(130);
    lcd.setCursor(0, 3);
    lcd.print("   DOOR CLOSING     ");
    opened = false;
}

// Function to move the motors
void MotorMove() {
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    motorL.step(1);
    motorR.step(1);
}

// Function to update the LCD display
void DisplayUpdate() {
    lcd.setCursor(0, 0);
    lcd.print("LIVES: ");
    lcd.setCursor(7, 0);
    lcd.print("LEVEL: ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(6, 0);
    lcd.print(life);
    lcd.setCursor(15, 0);
    lcd.print(level);
    lcd.setCursor(9, 1);
    lcd.write(1);  // Display heart icon for lives
}

// Functions to play sounds (to be implemented)
void LoseSound() {
    Serial.println("losing life...");
}

void SaveSound() {
    Serial.println("saving life...");
}

// Functions to color the LED rings
void colorWipe0(uint32_t c) {
    for (uint16_t i = 0; i < ring0.numPixels(); i++) {
        ring0.setPixelColor(i, c);
        ring0.show();
        delay(50);
    }
}
void colorWipe1(uint32_t c) {
    for (uint16_t i = 0; i < ring1.numPixels(); i++) {
        ring1.setPixelColor(i, c);
        ring1.show();
        delay(50);
    }
}
void colorWipe2(uint32_t c) {
    for (uint16_t i = 0; i < ring2.numPixels(); i++) {
        ring2.setPixelColor(i, c);
        ring2.show();
        delay(50);
    }
}
void colorWipe3(uint32_t c) {
    for (uint16_t i = 0; i < ring3.numPixels(); i++) {
        ring3.setPixelColor(i, c);
        ring3.show();
        delay(50);
    }
}
void colorWipe4(uint32_t c) {
    for (uint16_t i = 0; i < ring4.numPixels(); i++) {
        ring4.setPixelColor(i, c);
        ring4.show();
        delay(50);
    }
}
void colorWipe5(uint32_t c) {
    for (uint16_t i = 0; i < ring5.numPixels(); i++) {
        ring5.setPixelColor(i, c);
        ring5.show();
        delay(50);
    }
}
void colorWipe6(uint32_t c) {
    for (uint16_t i = 0; i < ring6.numPixels(); i++) {
        ring6.setPixelColor(i, c);
        ring6.show();
        delay(50);
    }
}