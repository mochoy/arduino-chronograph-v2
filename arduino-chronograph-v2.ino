#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Button.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define IR_REC_ONE_PIN 0                  //pins for IR Gate
#define IR_REC_TWO_PIN 1                  //pins for IR gate
#define GATE_DISPLACEMENT 0.22916666666   //distance between gate, in feet
#define IR_GATE_TRIP_VAL 60               //value at which the IR gate is considered "blocked", or "tripped"

#define UNIT_TOG_BTN_PIN 2
#define FPS 0
#define MPH 1
#define MPS 2
#define KMPH 3


Adafruit_SSD1306 display(4);

Button unitTogBtn (UNIT_TOG_BTN_PIN, true, true, 20);

double firstTripTime, secondTripTime;    //keep track of timing between IR gate breakage
boolean hasFirstTripped = false;    //flag to ensure proper timing

int currentUnit = 0;
double chronoReadings[5] = {0, 0, 0, 0, 0};
int units[5] = {0, 0, 0, 0, 0};
String trailingUnits[5] = {"ft/s", "mi/hr", "m/s", "km/hr"};
float unitConversion[5] = {1};

void setup () { 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //begin dispaly
  displayVals();                              //display vals
}

void loop () {
  toggUnits();
  chrono();   //check and do chrono
}

void toggUnits () {
  unitTogBtn.read();
  if (unitTogBtn.wasPressed()) {
    if (currentUnit == 3) {
      currentUnit = 0;
    } else {
      currentUnit++;
    }
  }
}

//chrono timing and trip checking
void chrono () {    
  if (!hasFirstTripped && (map(analogRead(IR_REC_ONE_PIN), 0, 1024, 0, 100) > IR_GATE_TRIP_VAL) ) {   
    firstTripTime = micros();
    hasFirstTripped = true;
  }
  
  if (hasFirstTripped && (map(analogRead(IR_REC_TWO_PIN), 0, 1024, 0, 100) > IR_GATE_TRIP_VAL) ) {   
    secondTripTime = micros();
    hasFirstTripped = false;
    calculateChronoReadings();
  }
}

//chrono calculations based on time
double calculateChronoReadings () {
  double chronoReading = (double)GATE_DISPLACEMENT/((secondTripTime - firstTripTime) / 1000000) * unitConversion[currentUnit];
  if (findFirstEmptyVal(true) == -1) {
    chronoReadings[0] = chronoReading;
  } else {
    chronoReadings[findFirstEmptyVal(true)] = chronoReading;
  }

  displayVals();       //display vals
}

int findFirstEmptyVal (bool isFromChronoReadings) {
  int valToReturn = -1;
  for (int i = 0; i < 5; i++) {
    if (isFromChronoReadings && chronoReadings[i] == 0) {
      valToReturn = i;
    } else if (!isFromChronoReadings && units[i] == 0) {
      valToReturn = i;
    }
  }

  return valToReturn;
}

//display text to screen
void displayVals () {    
    display.clearDisplay();                 //clear display
    display.setTextColor(WHITE);            //set text color for display

    display.setTextSize(3);                 //set text size for dispaly
    display.setCursor(0, 0);                //set cursor so display knows where to type 
    display.print((String)chronoReadings[0] + trailingUnits[currentUnit]);

    display.setTextSize(1);
    for (int i = 1; i < (findFirstEmptyVal(true) == -1 ? 5 : findFirstEmptyVal(true)); i++) {
      display.setCursor(5, (i * 8));
      display.print((String)chronoReadings[i] + trailingUnits[units[i]]);
    }

    display.display();                      //display chrono reading
}
