#include <avr/sleep.h>
#include <avr/power.h>
#include <ADCTouch.h>
#include <FastLED.h>

#define BRIGHTNESS  128        //Brightness modifier
#define LED_TYPE    WS2812B    //LED Chipset
#define COLOR_ORDER GRB        //LED color order
#define UPDATES_PER_SECOND 800 //refresh rate of LEDs

const int LEDPin         = 4;     //connected to signal pin on strip
const int LEDpwrPin      = 3;     //HIGH allows power to go to strip
const int btnPin         = 2;     //connected to pushbutton
const int chargeRead     = A0;     //pin that reads if prop is being charged
const int sensorPinUP    = A3;     //UP
const int sensorPinDOWN  = A2;     //Down
const int sensorPinLEFT  = A1;     //LEFT
const int sensorPinRIGHT = A4;     //RIGHT, Temporarily ENTER
const int NUM_LEDS       = 72;         //how many LEDs are in the strip

CRGB leds[NUM_LEDS];

boolean modeEnter = false;  //enter mode value

unsigned long timeI = 0; // Pushbutton value. Track time of button press

int maxModes = 7;           // This value is actually (number of modes - 1)
int mode = 4;               //Starting mode
int ENTERcount = 0;         //count for ENTER function
int UPcal, DOWNcal, LEFTcal, RIGHTcal;       //CapSense reference values to remove offset

//Rainbow March value
int thishue;

//Twinkle values
int ranamount = 72;                                           // The higher the number, lowers the chance for a pixel to light up.
uint8_t thisdelay = 50;                                       // Standard delay value in milliseconds.
uint8_t fadeval = 224;   // Fade rate

//Confetti values
uint8_t  thisfade = 8;                                        // How quickly does it fade? Lower = slower fade rate.
uint8_t   thisinc = 8;                                        // Incremental value for rotating hues
uint8_t   thissat = 100;                                      // The saturation, where 255 = brilliant colours.
uint8_t   thisbri = 255;                                      // Brightness of a sequence. Remember, max_bright is the overall limiter.
int       huediff = 256;                                      // Range of random #'s to use for hue




void setup()                    
{
   Serial.begin(9600);
   Serial.print("Turning on with mode ");
   Serial.println(mode);
   
   FastLED.addLeds<NEOPIXEL, LEDPin>(leds, NUM_LEDS);
   FastLED.setBrightness(BRIGHTNESS);
   
   UPcal = ADCTouch.read(sensorPinUP, 500);    //Touch Sensor calibration values
   DOWNcal = ADCTouch.read(sensorPinDOWN, 500);  
   LEFTcal = ADCTouch.read(sensorPinLEFT, 500);  
   RIGHTcal = ADCTouch.read(sensorPinRIGHT, 500);   

   pinMode(btnPin,INPUT);

   attachInterrupt (digitalPinToInterrupt(btnPin), interruptIR, LOW);
       
   pinMode(LEDPin,OUTPUT);
   pinMode(LEDpwrPin,OUTPUT);
   
   digitalWrite(LEDpwrPin,HIGH);
   
   leds[mode] = CRGB::Black;
   FastLED.show();
}

void sleepNow()
{ 
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Serial.println("Going to sleep");
  FastLED.clear();
  FastLED.show();
  digitalWrite(LEDpwrPin,LOW);
  delay(1000);
  attachInterrupt(digitalPinToInterrupt(btnPin), wakeUpNow, LOW);
  sleep_cpu(); // Go to sleep                              
  sleep_disable(); // Return from sleep
}

//run on Wakeup
void wakeUpNow(){
  detachInterrupt(0);
  attachInterrupt (digitalPinToInterrupt(btnPin), interruptIR, LOW); 
}


//pushbutton interrupt, read time button is pushed
void interruptIR() {
  timeI = millis();
  attachInterrupt(digitalPinToInterrupt(btnPin), interruptIF, HIGH);
}

//commands executed based on button press time
void interruptIF() {
  if ((millis() - timeI) >= 1000) {
    Serial.println("This is where I should be sleeping!");
    sleepNow();
  }
  else {
    attachInterrupt (digitalPinToInterrupt(btnPin), interruptIR, LOW);
    modeEnter = !modeEnter; 
    Serial.print("Toggle enter mode ");
    Serial.println(modeEnter);
  } 
}

//confetti code
void confetti() {                                             // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, thisfade);                    // Low values = slower fade.
  int pos = random16(NUM_LEDS);                               // Pick an LED at random.
  leds[pos] += CHSV((thishue + random16(huediff))/4 , thissat, thisbri);  // I use 12 bits for hue so that the hue increment isn't too quick.
  thishue = thishue + thisinc;                                // It increments here.
} // confetti()


void loop(){ 
  int RIGHTval = ADCTouch.read(sensorPinRIGHT);   //no second parameter
  RIGHTval -= RIGHTcal;     //read calibration

  
   if (RIGHTval > 60){
      ENTERcount ++;
    }

   if (ENTERcount == 10){
      Serial.println("Trying to sleep.");
      sleepNow();
      ENTERcount = 0;
    }
    
   if ((RIGHTval < 60) && (ENTERcount < 10) && (ENTERcount > 0)){
      ENTERcount = 0;
      modeEnter = !modeEnter; 
      Serial.print("Toggle enter mode ");
      Serial.println(modeEnter);
    }  
 
 
   if (modeEnter) {
      digitalWrite(LEDpwrPin,HIGH);
  
      int UPval = ADCTouch.read(sensorPinUP);
      int DOWNval = ADCTouch.read(sensorPinDOWN);
      UPval -= UPcal;
      DOWNval -= DOWNcal;    
    
    for(int LEDNumber = 0; LEDNumber < NUM_LEDS; LEDNumber++) {
      leds[LEDNumber] = CRGB::Blue;
      for(int modeNumber = 0; modeNumber <= mode; modeNumber++) {
      leds[modeNumber] = CRGB::Red;
      }
    FastLED.show();
    }

    
    if ((DOWNval > 40) && (mode == 0)) {   //If down pressed on mode 0, sets to max mode
      mode = maxModes;
      Serial.println("Mode Down");
      Serial.print("Now on mode ");
      Serial.println(mode);
      delay (200);
    }
        else if ((DOWNval > 40) && (mode > 0)) { //Go down a mode
      mode --;
      Serial.println("Mode Down");
      Serial.print("Now on mode ");
      Serial.println(mode);
      delay (200);
    }
    
    if ((UPval > 40) && (mode == maxModes)) {   //If up pressed on maxMode, set mode to 0
      mode = 0;
      Serial.println("Mode Up");
      Serial.print("Now on mode ");
      Serial.println(mode);
      delay (200);
    }
    else if ((UPval > 40) && (mode < maxModes)) { //Go Up a mode
      mode ++;
      Serial.println("Mode Up");
      Serial.print("Now on mode ");
      Serial.println(mode);
      delay (200);
        }
    }
    
    else {

 
    switch (mode) {
      case 0:                                   // red chaser down both staves
          for(int dot = 0; dot < NUM_LEDS; dot++) { 
          leds[dot] = CRGB::Red;
          FastLED.show();
          // clear this led for the next time around the loop
          leds[dot] = CRGB::Black;
          delay(30);
        }
      break;
      
      case 1:{                                                     //green chaser
         for(int dot = 0; dot < NUM_LEDS; dot++) { 
          leds[dot] = CRGB::Green;
          FastLED.show();
          // clear this led for the next time around the loop
          leds[dot] = CRGB::Black;
          delay(30);
         }
      break;
      }
      case 2:{                                                     // blue chaser
         for(int dot = 0; dot < NUM_LEDS; dot++) { 
          leds[dot] = CRGB::Blue;
          FastLED.show();
          // clear this led for the next time around the loop
          leds[dot] = CRGB::Black;
          delay(30);
         }
       break;
      }
       case 3:{                                                    //led mirror 
         for(int LEDNumber = 0; LEDNumber < (NUM_LEDS/2); LEDNumber++) { 
           leds[NUM_LEDS/2 - (LEDNumber + 1)] = CRGB::Purple;
           leds[NUM_LEDS/2 + LEDNumber] = CRGB::Purple;
           
           FastLED.show(); 
           
           delay(30);
                      
           leds[NUM_LEDS/2 - (LEDNumber + 1)] = CRGB::Black;
           leds[NUM_LEDS/2 + LEDNumber] = CRGB::Black;          
         }
        break;
       }
        case 4:{                                                   //rainbow March mode (wide)
          thishue++;                                               // Increment the starting hue.
          fill_rainbow(leds, NUM_LEDS, thishue, 10);               // Use FastLED's fill_rainbow routine.
           // rainbow_march()
          FastLED.show();
          break;
        }   
        case 5:{                                                   //rainbow march (narrow)
          thishue++;                                               // Increment the starting hue.
          fill_rainbow(leds, NUM_LEDS, thishue, 40);               // Use FastLED's fill_rainbow routine.
           // rainbow_march()
          FastLED.show();
        break;
        }
        case 6:{                                                   //twinkle mode        
          if (ranamount >NUM_LEDS) ranamount = NUM_LEDS;           // Make sure we're at least utilizing ALL the LED's.
            int idex = random16(0, ranamount);
          if (idex < NUM_LEDS) {                                   // Only the lowest probability twinkles will do.
            leds[idex] = random();                                 // The idex LED is set to a random 32 bit value
          }
          for (int i = 0; i <NUM_LEDS; i++) leds[i].nscale8(fadeval); // Go through the array and reduce each RGB value by a percentage.
          FastLED.show();
        break;
        }
        case 7:{                                                   //confetti mode
          confetti();    
          FastLED.show();
        break;
        }
      }
   }    
}
