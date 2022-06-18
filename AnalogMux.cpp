#include "AnalogMux.h"

AnalogMux::AnalogMux(int S0, int S1, int S2, int EN)
{
  this->selectPins[0] = S0;
  this->selectPins[1] = S1;
  this->selectPins[2] = S2;
  this->EN = EN;  
  
  pinMode(this->EN, OUTPUT);
  digitalWrite(this->EN, HIGH);

    for (int i=0; i<3; i++)
  {
    pinMode(this->selectPins[i], OUTPUT);
    digitalWrite(this->selectPins[i], LOW);
  }
}

void AnalogMux::selectMuxPin(int pin)
{

  if(pin == 255)
  {
    this->selectedOut = pin;
    digitalWrite(this->EN, HIGH);
  }
  if (pin > 7) 
  {
    this->selectedOut = 255;
    return; // Exit if pin is out of scope
  }
  
  digitalWrite(this->EN, LOW);
  this->selectedOut = pin;
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(this->selectPins[i], HIGH);
    else
      digitalWrite(this->selectPins[i], LOW);
  }
}

int AnalogMux::getSelectedOut()
{
  return this->selectedOut;
}
