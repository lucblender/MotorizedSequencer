#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN        2
#define NUMPIXELS 11

#define ONOFFMODE_LED 10
#define ACTIVESTEPMODE_LED 9

#define SPEED_LED 8

enum ButtonMode {ONOFF, ACTIVESTEP};

ButtonMode buttonMode = ONOFF;

int BPM = 200;
float delayPeriod = 1000.0/(BPM/60);
const unsigned long loopPeriod = 1; //milliseconds
static unsigned long lastLoop;

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_RGBW + NEO_KHZ800);

uint32_t  redColor = pixels.Color(0, 150, 0);
uint32_t  offColor = pixels.Color(0, 0, 0);

void setup() {
  Serial.begin(9600);
  Serial.println(delayPeriod);
  // put your setup code here, to run once:
  pixels.begin();
  pixels.clear(); // Set all pixel colors to 'off'
  
  updateMode();

}

void loop() {
  
  blinkSpeed();
  
  while(millis()-lastLoop < delayPeriod) {
  }
  lastLoop = millis();
}

void updateMode()
{
  if(buttonMode == ONOFF)
  {    
    pixels.setPixelColor(ONOFFMODE_LED, redColor);
    pixels.setPixelColor(ACTIVESTEPMODE_LED, offColor);
  }
  else
  {
    pixels.setPixelColor(ONOFFMODE_LED, offColor);
    pixels.setPixelColor(ACTIVESTEPMODE_LED, redColor);  
  }
  pixels.show();
}

void blinkSpeed()
{
    pixels.setPixelColor(SPEED_LED, redColor); 
    pixels.show();
    delay(1); 
    pixels.setPixelColor(SPEED_LED, offColor);
    pixels.show();
}
