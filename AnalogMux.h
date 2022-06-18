//--------------------------------------
//
//  Based on 74HC4051
//
//--------------------------------------

#ifndef _ANALOG_MUX_H
#define _ANALOG_MUX_H

#include <Arduino.h>

class AnalogMux{

  public:
  AnalogMux(int S0, int S1, int S2, int EN);
  void selectMuxPin(int pin);
  int getSelectedOut();
  

  private:
  int selectPins[3] = {0,0,0};
  int EN = 0;
  int selectedOut = 255;

  
};


#endif
