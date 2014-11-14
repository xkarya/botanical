//using the neopixel library, available from adafruit.com
#include <Adafruit_NeoPixel.h>
//rgb led strip of 3 leds, from pin 10, etc.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, 10, NEO_GRB + NEO_KHZ800);

//pins
int powerPin = 2;
int buttonPin = 12;
int satPin = 14;
int photoPin = 15;
int thermPin = 16;

//thresholds
int tempHigh = 65; //max comfortable temp
int tempLow = 32; //lowest okay temp
int photoHigh = 1000; //direct sunlight
int photoLow = 50; //darkness, more or less
int satHigh = 700; //max saturation, is inverted scale
int satLow = 950; //very very dry

//misc.
float steinhart; //for steinhart equation
int outputMax = 100; //for mapping analog inputs, from 0 to whatever this value is
int color[] = {0,0,0}; //red, green, blue, for rgb led indicators
int brightness[] = {255,120,60}; //different levels, for rgb led indicators
int brightnessC = 0; //which level it should be on, changes when button is pressed
boolean toggle;

void setup(){
  //Serial.begin(9600); //for diagnostic
  //powerPin
  pinMode(powerPin,OUTPUT);
  digitalWrite(powerPin,HIGH);
  //button pin
  pinMode(buttonPin,INPUT);
  //neopixel
  strip.begin();
};
void loop(){
  //printAll(); //for diagnostic
  lights(); //updates indicator lights
  //buttons, turns push button into toggle button
  if (digitalRead(buttonPin) == HIGH && toggle == false){
    toggle = true;
    brightnessC++;
    if (brightnessC > 2){
      brightnessC = 0;
    };
  };
   if (digitalRead(buttonPin) == LOW && toggle == true){
    toggle = false;
   };
};
//temperature function, scales between 0 and 100
int thermOut(){
  int temp;
  float Rt;
  int bco = 4000; //b coefficient
  int output;
  //stienhart equation
  temp = analogRead(thermPin);
  Rt = 47200/((float)1032/temp - 1);
  steinhart = Rt / 50000;
  steinhart = log(steinhart);
  steinhart /= bco;
  steinhart += 1.0 / (25 + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15; //from kelvin to C
  steinhart =  steinhart*9/5+32; //to convert from C to F
  //this equation is correct, for a 10k thermistor with a 10k resistor and a 5v pin, 
  //but for some reason the output on my cheat arduino knockoff isn't supplying enough power
  //consequently, the temperature yielded is consistently 10 degrees f higher than
  //what it should be, so...
  steinhart = steinhart - 10; //to fix the problem
  //function output, from 0 to 100 depending on conditions
  //okay conditions
  if (steinhart >= tempHigh){
    output = outputMax;
  };
  //okay conditions
  if (steinhart < tempHigh && steinhart > tempLow){
    output = map(steinhart, tempLow, tempHigh, 0, outputMax);
  };
  //bad conditions
  if (steinhart <= tempLow){
    output = 0;
  };
  return output;
};
//light function, scales between 0 and 100
int photoOut(){ //light function, scales between 0 and 100;
  int photo = analogRead(photoPin);
  int output;
  //output
  //good conditions
  if (photo >= photoHigh){
    output = outputMax;
  };
  //okay conditions
  if (photo < photoHigh && photo > photoLow){
    output = map(photo, photoLow, photoHigh, 0, outputMax);
  };
  //bad conditions
  if (photo <= photoLow){
    output = 0;
  };
  return output;
};
//function output, from 0 to 100 depending on conditions
//water saturation conditions from hygrometer, scales between 0 and 100
int satOut(){
  int sat = analogRead(satPin);
  int output;
  //output
  //good conditions
  if (sat < satHigh){
    output = outputMax;
  };
  //okay condiditons
  if (sat > satHigh && sat < satLow){
    output = map(sat, satHigh, satLow, outputMax, 0); //from max to 0 this time, because
    //scale is inversed.  high value being poor conductivity, low value meaning good
    //conductivity
  };
  //bad conditions
  if (sat > satLow){
    output = 0;
  };
  return output;
};
//maps input to color from red (poor conditions) to green (good conditions)
int colorMap(int input){
  //temp = temporary, not temperature.  first needs the 0 to 100 outputs scaled to 0 to 510 (2*255) 
  //in order to facilitate rgb transitions.  i know this step could be skipped, by scaling to
  //to 510 at the end of each (therm, sat, and photo) function, but i'm too lazy and will fix it later.
  int temp = map(input, 0, outputMax, 0, 510);
  //if the conditions are very bad, it'll be just red.  greener and greener as the conditions get better
  if (temp <= 255){
    color[0] = 255; //red
    color[1] = temp; //green
    color[2] = 0; //blue
  };
  //less and less red as the conditions get better.  if the conditions are very good, it'll be just green.
  if (temp > 255){
    color[0] = 255-(temp - 255); //red
    color[1] = 255; //green
    color[2] = 0; //blue
  };
};
//print all inputs, for diag purposes.
void printAll(){
//with a small delay because otherwise the serial port is too crazy
  Serial.println("sat");
  Serial.println(analogRead(satPin));
  Serial.println(satOut());
  Serial.println("photo");
  Serial.println(analogRead(photoPin));
  Serial.println(photoOut());
  Serial.println("therm");
  Serial.println(thermOut());
  Serial.println(steinhart);
  delay(500);
;};
//update indicator function
void lights(){
  //saturation
  colorMap(satOut());
  strip.setPixelColor(2, color[0], color[1], color[2]);
  //photo
  colorMap(photoOut());
  strip.setPixelColor(1, color[0], color[1], color[2]);
  //therm
  colorMap(thermOut());
  strip.setPixelColor(0, color[0], color[1], color[2]);
  strip.setBrightness(brightness[brightnessC]);
  strip.show();
};
