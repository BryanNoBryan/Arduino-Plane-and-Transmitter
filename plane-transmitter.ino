#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define LX A0
#define LY A1
#define LBut 4
#define RX A2
#define RY A3
#define RBut 3

//joysticks
int leftx;
int lefty;
int rightx;
int righty;

//joystick buttons
bool leftPressed;
bool rightPressed;


//nRF24L01 things
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "bryan";

struct Data_Pack {
  byte leftx;
  byte lefty;
  bool leftb;
  byte rightx;
  byte righty;
  bool rightb;
} data;


void setup() {
  Serial.begin(9600);
  pinMode(LBut, INPUT_PULLUP);
  pinMode(RBut, INPUT_PULLUP);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  Serial.println("startup");
}

void loop() {
  //setup specifics
  leftx = 1023 - analogRead(LX);
  lefty = analogRead(LY);
  rightx = analogRead(RX);
  righty = 1023 - analogRead(RY);
  

  //The input is pullup-ed, so 1 is default false
  leftPressed = digitalRead(LBut) == 1 ? false : true;
  rightPressed = digitalRead(RBut) == 1 ? false : true;
  
  //value insurance
  leftx = map(constrain(leftx, 0, 1023), 0, 1023, 0, 255);
  lefty = map(constrain(lefty, 0, 1023), 0, 1023, 0, 255);
  rightx = map(constrain(rightx, 0, 1023), 0, 1023, 0, 255);
  righty = map(constrain(righty, 0, 1023), 0, 1023, 0, 255);

  //struct ing
  data.leftx =    leftx;
  data.lefty =    lefty;
  data.rightx =   rightx;
  data.righty =   righty;
  data.leftb =  leftPressed;
  data.rightb =  rightPressed;

  radio.write(&data, sizeof(data));
  delay(50);
  // Serial.println("written");
  // Serial.println(data.leftx);
  //   Serial.println(data.lefty);
  //   Serial.println(data.leftb);
  //   Serial.println(data.rightx);
  //   Serial.println(data.righty);
  //   Serial.println(data.rightb);
  //   delay(200);
}
