//     URL: https://github.com/RobTillaart/Vibration


#include "Vibration.h"

//system states
#define NORMAL    0
#define WARNING   1
#define EMERGENCY 2

// tilt sensor states
#define UP        3
#define DOWN      4

// tilt pin
#define TILT      5

//light pin
#define GREEN     6
#define YELLOW    7
#define RED       8

//activity threshold
#define IDLE    100
#define UNUSUAL 250

VibrationSensor VBS(A0);

uint32_t previous_avr;
uint32_t current_avr;
uint32_t unusual_avr;
int counter; //counts until emergency
int counter2; //counts until back to normal
int user_state; //for normal/warning/emergency

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("VIBRATION_LIB_VERSION: ");
  Serial.println(VIBRATION_LIB_VERSION);
  Serial.println();

  pinMode(TILT, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT); 
  pinMode(RED, OUTPUT); 

  previous_avr = 0;
  current_avr = 0;
  unusual_avr = 0;
  counter = 0;
  counter2 = 0;
  user_state = NORMAL;
}


void loop()
{
  //  measure for one second
  VBS.measure(1000000);
  //  average with one decimal
  Serial.print("Samples: \t");
  Serial.print(VBS.sampleCount());
  Serial.print("\t avg: \t");
  current_avr = VBS.average();
  Serial.print(current_avr);
  Serial.println();

  /**
  Resting value: typically < 100
  Walking value: typically 100 - 250

  Unusual vibration value should be approx 250+ (assume bodyweight impact w/ surface)
  check also user orientation (did they switch from vertical-horizontal or vice versa?)
  */

  switch(user_state) {
    case NORMAL:
      digitalWrite(GREEN, HIGH);
      //detect unusual vibration
      if (current_avr >= UNUSUAL) {
        Serial.print("Warning! Unusual vibration detected \n");
        user_state = WARNING;
        unusual_avr = current_avr;
      }
      break;
    case WARNING:
      digitalWrite(GREEN, LOW);
      digitalWrite(YELLOW, HIGH);

      //check if user is also horizontal
      if (current_avr <= IDLE && current_avr == previous_avr && counter < 5 && digitalRead(TILT) == LOW) {
          Serial.print("The user may be unconscious... ");
          Serial.print(counter);
          Serial.print(" seconds elapsed before contacting emergency");
          counter++;
      }
      else {
        counter2++;
      }
      //check if 5 counts of inactivity after impact has elapsed before notifying
      if (counter == 5) {
        user_state = EMERGENCY;
      }
      else if (counter2 == 10) { //assume activity resumed so no fainting
        digitalWrite(YELLOW, LOW);
        Serial.print("No problem. Back to normal \n");
        counter = 0;
        counter2 = 0;
        user_state = NORMAL;
      }  
      break;
    case EMERGENCY:
      digitalWrite(YELLOW, LOW);
      digitalWrite(RED, HIGH);
      Serial.print("HELP!! EMERGENCY!! \n");
      break;
  }
  previous_avr = current_avr;
}


//  -- END OF FILE --
