#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <Wire.h>
#include "AnalogMux.h"

#include "grove_alphanumeric_display.h"
#include <MX1508.h>

// MAIN DEFINITION
#define SEQUENCE_LEN 8

// EEPROM DEFINITION
#define PROGRAM_NUMBER 8
unsigned long arrayValue[PROGRAM_NUMBER][SEQUENCE_LEN] = {
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
  {500, 500, 500, 500, 500, 500, 500, 500},
};

// LEDS
#define NEOPIXEL_PIN 16
#define NUMPIXELS 11

//ON OFF MODE LEDS
#define ONOFFMODE_LED 8
#define ACTIVESTEPMODE_LED 9

//SPEED BPM LED
#define BPM_LED 10
// Buttons
#define BTN0 40
#define BTN1 33
#define BTN2 34
#define BTN3 35
#define BTN4 36
#define BTN5 37
#define BTN6 38
#define BTN7 39
int BTNs [SEQUENCE_LEN] = {BTN0, BTN1, BTN2, BTN3, BTN4, BTN5, BTN6, BTN7};

int BTNState [SEQUENCE_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};
int BTNLastState [SEQUENCE_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

// UIs
#define LOAD_BTN 64
#define SAVE_BTN 65
#define PLAY_BTN 14
#define PLAY_LED 15
#define MODE_BTN 23
#define SELECT_POT A9
#define BPM_POT A8

#define BPM_MIN 56
#define BPM_MAX	241
#define BPM_RANGE (BPM_MAX-BPM_MIN)

int loadBtnState = 0;
int loadBtnLastState = 0;
int saveBtnState = 0;
int saveBtnLastState = 0;
int playBtnState = 0;
int playBtnLastState = 0;
int modeBtnState = 0;
int playStop = 0;
int modeBtnLastState = 2;
int programValue = 0;
int oldProgramValue = -1;

int bpmValue = BPM_MIN;
int bpmLastValue = 0;

// Sync
#define SYNC_IN 18
#define SYNC_OUT 19

//CVs gate
#define GATE 22
#define GATE_RELEASE_TIME 200

bool gateReleased = false;

#define S0_CV 25
#define S1_CV 26
#define S2_CV 27
#define EN_CV 24

AnalogMux cvMux(S0_CV, S1_CV, S2_CV, EN_CV);

//MOTORS DEFINITION
#define MOT0_PINA 2
#define MOT0_PINB 3

#define MOT1_PINA 4
#define MOT1_PINB 5

#define MOT2_PINA 6
#define MOT2_PINB 7

#define MOT3_PINA 8
#define MOT3_PINB 9

#define MOT4_PINA 10
#define MOT4_PINB 11

#define MOT5_PINA 12
#define MOT5_PINB 13

#define MOT6_PINA 44
#define MOT6_PINB 45

#define MOT7_PINA 52
#define MOT7_PINB 53

MX1508 motor0(MOT0_PINA, MOT0_PINB, FAST_DECAY, 2);
MX1508 motor1(MOT1_PINA, MOT1_PINB, FAST_DECAY, 2);
MX1508 motor2(MOT2_PINA, MOT2_PINB, FAST_DECAY, 2);
MX1508 motor3(MOT3_PINA, MOT3_PINB, FAST_DECAY, 2);
MX1508 motor4(MOT4_PINA, MOT4_PINB, FAST_DECAY, 2);
MX1508 motor5(MOT5_PINA, MOT5_PINB, FAST_DECAY, 2);
MX1508 motor6(MOT6_PINA, MOT6_PINB, FAST_DECAY, 2);
MX1508 motor7(MOT7_PINA, MOT7_PINB, FAST_DECAY, 1); //only motor with 1 pwm

MX1508 motors[SEQUENCE_LEN] = {motor0, motor1, motor2, motor3, motor4, motor5, motor6, motor7};
bool motorsReached[SEQUENCE_LEN] = {false, false, false, false, false, false, false, false};

bool motorReachStart = false;
int motorProgramValue = 0;

//POT ANALOGIN DEFINITION
#define POT0_IN A0
#define POT1_IN A1
#define POT2_IN A2
#define POT3_IN A3
#define POT4_IN A4
#define POT5_IN A5
#define POT6_IN A6
#define POT7_IN A7
int POT_INs [SEQUENCE_LEN] = {POT0_IN, POT1_IN, POT2_IN, POT3_IN, POT4_IN, POT5_IN, POT6_IN, POT7_IN};
int analogValues [SEQUENCE_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

#define DELTA_ANALOG_VALUE 30

#define PWM 255
#define PWM_MEDIUM 100
#define PWM_MEDIUM_7 150
#define PWM_LOW 65
#define PWM_LOW_7 120

// 12 segments
Seeed_Digital_Tube tube;
// Keeps track of the current stage of tube 2's animation.
int currentSegment = 0;

// The frames of tube 2's animation.
uint16_t tubeFrames[] = {
  SEGMENT_TOP,
  SEGMENT_TOP_LEFT,
  SEGMENT_TOP_LEFT_DIAGONAL,
  SEGMENT_TOP_VERTICAL,
  SEGMENT_TOP_RIGHT_DIAGONAL,
  SEGMENT_TOP_RIGHT,
  SEGMENT_MIDDLE_LEFT,
  SEGMENT_MIDDLE_RIGHT,
  SEGMENT_BOTTOM_LEFT,
  SEGMENT_BOTTOM_LEFT_DIAGONAL,
  SEGMENT_BOTTOM_VERTICAL,
  SEGMENT_BOTTOM_RIGHT_DIAGONAL,
  SEGMENT_BOTTOM_RIGHT,
  SEGMENT_BOTTOM
};

int gateState [SEQUENCE_LEN] = {1, 1, 1, 1, 1, 1, 1, 1};
int activeStepState [SEQUENCE_LEN] = {1, 1, 1, 1, 1, 1, 1, 1};

int stepIndex = 0;

enum ButtonMode {ONOFF, ACTIVESTEP};

ButtonMode buttonMode = ONOFF;

float delayPeriod = 1000.0 / (bpmValue / 60);
const unsigned long loopPeriod = 1; //milliseconds
static unsigned long lastLoop;

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_RGBW + NEO_KHZ800);

uint32_t  redColor = pixels.Color(0, 150, 0);
uint32_t  gateColor = pixels.Color(15, 150, 0);
uint32_t  activeStepColor = pixels.Color(0, 150, 5);
uint32_t  offColor = pixels.Color(0, 0, 0);

// Sync linked in variables
#define LOWEST_SYNC_BPM 30
static unsigned long lastSync = 0;
static unsigned long delaySinceLastSync = 0;
float estimatedSyncPeriod = 0;
float maxSyncPeriod = 1000.0 / (LOWEST_SYNC_BPM / 60.0);
bool syncConnected = false;
bool lastSyncConnected = false;

bool reached = false;
int setpoint = random(0, 1024);


//TODO 
#define PROGRAM_CIRCULAR_BUFFER_LEN 32
int programValueBuffer [PROGRAM_CIRCULAR_BUFFER_LEN]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int indexProgramValueBuffer = 0;

int sensorValue = 0;    //initialization of sensor variable, equivalent to EMA Y
float EMA_a = 0.1;      //initialization of EMA alpha
int EMA_S = 0;   

void setup() {
  Serial.begin(9600);
  Serial.println(delayPeriod);

  for (int i = 0; i < PROGRAM_NUMBER; i++) {
    // uncomment only one time to setup
    //saveValue(i);
    loadValue(i);
  }

  for (int i = 0; i < PROGRAM_NUMBER; i++) {
    Serial.print("Program ");
    Serial.print(i);
    Serial.print(" : ");
    for (int seq = 0; seq < SEQUENCE_LEN; seq++) {
      Serial.print(arrayValue[i][seq]);
      Serial.print(" ");
    }
    Serial.println("");
  }

  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    pinMode(BTNs[i], INPUT_PULLUP);
  }

  pinMode(SYNC_IN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SYNC_IN), syncInInterrupt, RISING);

  cvMux.selectMuxPin(255);

  pinMode(LOAD_BTN, INPUT_PULLUP);
  pinMode(SAVE_BTN, INPUT_PULLUP);
  pinMode(PLAY_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(PLAY_LED, OUTPUT);

  pinMode(SYNC_IN, INPUT_PULLUP);
  pinMode(SYNC_OUT, OUTPUT);
  digitalWrite(SYNC_OUT, LOW);
  pinMode(GATE, OUTPUT);
  digitalWrite(GATE, HIGH);

  Wire.begin();
  // If using four digital tubes, use this configuration.
  tube.setTubeType(TYPE_2, TYPE_2_DEFAULT_I2C_ADDR);

  tube.setBrightness(255);
  tube.setBlinkRate(BLINK_OFF);

  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    motors[i].stopMotor();
  }

  // put your setup code here, to run once:
  pixels.begin();
  pixels.clear(); // Set all pixel colors to 'off'

  for (int i = 0; i < SEQUENCE_LEN * 2; i++)
  {
    pixels.setPixelColor(i % SEQUENCE_LEN, pixels.Color(0, i * 15 + 15, 0));
    pixels.show();
    displayTube();
    tube.display();
  }

  tube.displayString("LX");
  pixels.clear(); // Set all pixel colors to 'off'
  updateMode();
  updateStatusLeds();
}

void loop() {



  if (motorReachStart == true)
  {
    bool reached = true;
    for (int i = 0; i < SEQUENCE_LEN; i++)
    {
      if (motorsReached[i] == false) {
        //reachSetpointSpeedRegulation(MX1508 motor, int AN_PIN, int setpoint)
        motorsReached[i] = reachSetpointSpeedRegulation(motors[i], POT_INs[i], (int)arrayValue[motorProgramValue][i]);
        reached &= motorsReached[i];
      }
    }
    if (reached == true)
    {
      motorReachStart = false;
    }
  }
  else
  {
    selectRead();
    getBpm();
    getModeBtn();
    scanBtns();
    getPlayStop();
    getLoadSaveBtn();
  }

  //sync in logic start
  if (millis() - lastSync > maxSyncPeriod)
  {
    syncConnected = false;
  }
  else
  {
    syncConnected = true;
  }

  if (lastSyncConnected != syncConnected)
  {
    if (syncConnected)
    {
      tube.clearBuf();
      tube.setTubeSingleChar(FIRST_TUBE, 'S');
      tube.setTubeSingleChar(SECOND_TUBE, 'I');
      tube.display();
    }
    else
    {
      //Put bpm to 0 to force re-displaying bpm
      bpmLastValue = 0;
    }
  }

  lastSyncConnected = syncConnected;
  //sync in logic end

  if (syncConnected == true && (millis() - lastSync) > (delaySinceLastSync - GATE_RELEASE_TIME) && gateReleased == false)
  {
    gateRelease();
  }
  if (syncConnected == false && ((millis() - lastLoop) > (delayPeriod - GATE_RELEASE_TIME)) && gateReleased == false)
  {
    gateRelease();
  }

  if (millis() - lastLoop > delayPeriod) {
    if (syncConnected == false)
    {
      doSequenceStep();
    }
    lastLoop = millis();
  }
}

void syncInInterrupt()
{
  delaySinceLastSync =  millis() - lastSync;
  doSequenceStep();
  lastSync = millis();
}

void gateSet()
{
  digitalWrite(GATE, LOW);
  gateReleased = false;
}

void gateRelease()
{
  digitalWrite(GATE, HIGH);
  gateReleased = true;
}


void getLoadSaveBtn()
{

  loadBtnState = !digitalRead(LOAD_BTN);
  saveBtnState = !digitalRead(SAVE_BTN);

  if (loadBtnState != loadBtnLastState && loadBtnState == 1)
  {
    loadValue(programValue);
    motorReachStart = true;
    motorProgramValue = programValue;

    for (int i = 0; i < SEQUENCE_LEN; i++)
    {
      motorsReached[i] = false;
    }
    tube.displayString("ld", 0);
  }

  if (saveBtnState != saveBtnLastState && saveBtnState == 1 && motorReachStart == false)
  {

    Serial.println("Save pressed");
    //force re-read of program to display it
    oldProgramValue = 0;
    selectRead();
    updateAnalogArray();
    saveValue(programValue);

    tube.displayString("SV", 0);

  }

  loadBtnLastState = loadBtnState;
  saveBtnLastState = saveBtnState;
}

void updateAnalogArray()
{
  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    analogValues[i] = analogRead(POT_INs[i]);
  }
}
void doSequenceStep()
{
  sendCvOut();
  sendSyncOut();
  blinkSpeed();
}

void updateMode()
{
  if (buttonMode == ONOFF)
  {
    pixels.setPixelColor(ONOFFMODE_LED, gateColor);
    pixels.setPixelColor(ACTIVESTEPMODE_LED, offColor);
  }
  else
  {
    pixels.setPixelColor(ONOFFMODE_LED, offColor);
    pixels.setPixelColor(ACTIVESTEPMODE_LED, activeStepColor);
  }
  pixels.show();
}

void sendSyncOut()
{
  digitalWrite(SYNC_OUT, !digitalRead(SYNC_OUT));
}

void blinkSpeed()
{
  pixels.setPixelColor(BPM_LED, redColor);
  pixels.show();
  delay(1);
  pixels.setPixelColor(BPM_LED, offColor);
  pixels.show();
}

bool reachSetpointSpeedRegulation(MX1508 motor, int AN_PIN, int setpoint)
{
  int speed = PWM;
  int pot = analogRead(AN_PIN);
  if (abs(pot - setpoint) > 20) {
    if (abs(pot - setpoint) < 100) {
      AN_PIN != POT7_IN ? speed = PWM_LOW : speed = PWM_LOW_7;
    }
    else if (abs(pot - setpoint) < 200) {
      AN_PIN != POT7_IN ? speed = PWM_MEDIUM : speed = PWM_MEDIUM_7;
    }
    else {
      speed = PWM;
    }
    if (pot < setpoint)
      motor.motorGo(speed);
    else
      motor.motorGo(-speed);
    return false;
  }
  else
  {
    motor.stopMotor();
    return true;
  }
}

void getBpm() {
  EMA_S = (EMA_a*analogRead(BPM_POT)) + ((1-EMA_a)*EMA_S);
  float tmpBpmValue = (EMA_S / 1024.0 * BPM_RANGE) + BPM_MIN;
  bpmValue = int(tmpBpmValue);;

  if (abs(bpmLastValue - bpmValue) > 2 )
  {
    delayPeriod = 1000.0 / (bpmValue / 60.0);
    if (syncConnected == false)
    {
      int hundred = bpmValue / 100;
      int ten = (bpmValue / 10) % 10;
      int unit = bpmValue % 10;
      tube.clearBuf();
      tube.setTubeSingleNum(FIRST_TUBE, ten);
      tube.setTubeSingleNum(SECOND_TUBE, unit);
      if (hundred == 0)
      {
        tube.setPoint(false, false);
      }
      else if (hundred == 1)
      {
        tube.setPoint(false, true);
      }
      else if (hundred == 2)
      {
        tube.setPoint(true, true);
      }
      tube.display();
    }
    else
    {
      //Show we in sync in
      tube.clearBuf();
      tube.setTubeSingleChar(FIRST_TUBE, 'S');
      tube.setTubeSingleChar(SECOND_TUBE, 'I');
      tube.display();
    }

    bpmLastValue = bpmValue;
  }
}

void selectRead() {
  //TODO
  programValueBuffer[indexProgramValueBuffer] = analogRead(SELECT_POT) / 128;
  indexProgramValueBuffer = (indexProgramValueBuffer+1) %PROGRAM_CIRCULAR_BUFFER_LEN;
  programValue = 0;
  for(int i = 0; i<PROGRAM_CIRCULAR_BUFFER_LEN; i++)
  {
    programValue += programValueBuffer[i];
  }
  programValue = programValue>>5;

  if (programValue != oldProgramValue)
  {
    tube.clearBuf();
    tube.setTubeSingleChar(FIRST_TUBE, 'P');
    tube.setTubeSingleNum(SECOND_TUBE, programValue);
    tube.display();
  }
  oldProgramValue = programValue;
}

void getModeBtn() {
  modeBtnState = digitalRead(MODE_BTN);
  if (modeBtnState != modeBtnLastState)
  {
    modeBtnState == 0 ? buttonMode = ONOFF : buttonMode = ACTIVESTEP;
    updateMode();
    updateStatusLeds();
  }
  modeBtnLastState = modeBtnState;
}

void sendCvOut()
{
  if (playStop == 1)
  {
    int jump = 0;
    stepIndex  = (stepIndex + 1) % SEQUENCE_LEN;

    while (activeStepState[stepIndex] == 0 && jump < SEQUENCE_LEN)
    {
      stepIndex  = (stepIndex + 1) % SEQUENCE_LEN;
      jump += 1;
    }

    if (gateState[stepIndex] == 1 && jump < SEQUENCE_LEN)
    {
      cvMux.selectMuxPin(stepIndex);
      gateSet();
    }
    else
    {
      cvMux.selectMuxPin(255);
      gateRelease();
    }
  }
}

void scanBtns() {
  //Get buttons
  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    BTNState[i] = !digitalRead(BTNs[i]);
  }
  //Check if changed
  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    if (BTNLastState[i] != BTNState[i] && BTNState[i] == 1)
    {
      if (buttonMode == ONOFF)
      {
        gateState[i] = ! gateState[i];
      }
      else
      {
        activeStepState[i] = ! activeStepState[i];
      }
      updateStatusLeds();
    }
  }
  //update last state
  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    BTNLastState[i] = BTNState[i];
  }
}

void getPlayStop()
{

  playBtnState = !digitalRead(PLAY_BTN);
  if (playBtnState != playBtnLastState && playBtnState == 1)
  {
    playStop = ! playStop;
    if (playStop == 0)
    {

      cvMux.selectMuxPin(255);
      digitalWrite(PLAY_LED, LOW);
    }
    else
    {
      digitalWrite(PLAY_LED, HIGH);
      stepIndex = SEQUENCE_LEN - 1;
    }
  }
  playBtnLastState = playBtnState;
}

void updateStatusLeds()
{
  if (buttonMode == ONOFF)
  {
    for (int i = 0; i < SEQUENCE_LEN; i++)
    {
      gateState[i] == 1 ? pixels.setPixelColor(i, gateColor) : pixels.setPixelColor(i, offColor);
    }
  }
  else
  {
    for (int i = 0; i < SEQUENCE_LEN; i++)
    {
      activeStepState[i] == 1 ? pixels.setPixelColor(i, activeStepColor) : pixels.setPixelColor(i, offColor);
    }
  }
  pixels.show();
}

// Display an animation going through all 14 segments in turn.
void displayTube() {
  // Increment tube 2's animation frame.
  currentSegment += 1;

  // Restart the animation if it has finished.
  if (currentSegment >= 14) {
    currentSegment = 0;
  }

  // Display the current frame of tube 2's animation.
  tube.setTubeSegments(FIRST_TUBE, tubeFrames[currentSegment]);
  tube.setTubeSegments(SECOND_TUBE, tubeFrames[currentSegment]);
}

void saveValue(int program) {

  for (int i = 0; i < SEQUENCE_LEN; i++)
  {
    arrayValue[program][i] = analogValues[i];
  }
  for (int i = 0; i < SEQUENCE_LEN; i++) {

    unsigned long lsb = arrayValue[program][i] & 0xFF;
    unsigned long msb = (arrayValue[program][i] >> 8) & 0xFF;
    EEPROM.write(program * (SEQUENCE_LEN * 2) + (i * 2), msb);
    EEPROM.write(program * (SEQUENCE_LEN * 2) + (i * 2) + 1, lsb);
  }
}

void loadValue(int program) {
  for (int i = 0; i < SEQUENCE_LEN; i++) {
    unsigned long msb = EEPROM.read(program * (SEQUENCE_LEN * 2) + (i * 2));
    unsigned long lsb = EEPROM.read(program * (SEQUENCE_LEN * 2) + (i * 2) + 1);
    arrayValue[program][i] = (msb << 8) + lsb;
  }
}
