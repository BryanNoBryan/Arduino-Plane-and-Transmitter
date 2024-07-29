#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include<Wire.h>
#include <Adafruit_PWMServoDriver.h>

//default
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  500 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

Servo mtr; //throttle
uint8_t leftail = 0;
uint8_t rightail = 1;
uint8_t elevator = 2;
uint8_t rudder = 3;

//initialization to ensure the safety sequence of the esc is completed
bool INITIALIZED = false;
//use this to know whether we turned off, then on again to avoid the sequence again
bool REINITIALIZATION = false;

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "bryan";

struct Data_Pack {
  byte leftx;
  byte lefty;
  bool leftb;
  byte rightx;
  byte righty;
  bool rightb;
} data, previousdata;
//two structs to do rising edge detection

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  Wire.begin();

  pwm.begin();
  //I don't have an oscilloscope to tune this
  // pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  mtr.attach(6);

  delay(10);
  Serial.println("ready up");
}


void loop() {
  if (radio.available()) {
    Serial.println("new data");
    radio.read(&data, sizeof(data));
    mapTo180(data);
    // Serial.println(data.leftx);
    // Serial.println(data.lefty);
    // Serial.println(data.leftb);
    // Serial.println(data.rightx);
    // Serial.println(data.righty);
    // Serial.println(data.rightb);
    
    //rising edge detector for two buttons
    if (data.leftb == 1 && data.rightb == 1 && (previousdata.leftb != 1 || previousdata.rightb != 1)) {
      Serial.println("triggering on/off");
      INITIALIZED = !INITIALIZED;
      if (INITIALIZED && !REINITIALIZATION) {
        //esc initialization sequence
        //0 - 180
        Serial.println("initialization sequence");
        Serial.println("170");
        mtr.write(180);
        delay(5000);
        Serial.println("40 startup");
        mtr.write(40);
        delay(5000);
        REINITIALIZATION = true;
      } else {
        mtr.write(0);
      }
    }

    //control surfaces, the -30 added are just minute adjustments accounting for physical discrepancies
    pwm.setPWM(leftail, 0, map(180-data.rightx, 0, 180, SERVOMIN-20, SERVOMAX-20));
    pwm.setPWM(rightail, 0, map(180-data.rightx, 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(elevator, 0, map(180-data.righty, 0, 180, SERVOMIN, SERVOMAX));
    //ignoring rudder for now
    //pwm.setPWM(elevator, 0, map(data.leftx, 0, 180, SERVOMIN, SERVOMAX));

    //throttle
    if (INITIALIZED) {
      Serial.println(data.lefty);
      Serial.println(mtrTrim(data.lefty));
      mtr.write(mtrTrim(data.lefty));
    }
    
    //make a copy to hold states
    previousdata = data;
  } else {
    // Serial.println("not it");
  }
}

//easier to work with
void mapTo180(Data_Pack& data) {
  data.leftx  = map(data.leftx , 0, 255, 0, 180);
  data.lefty  = map(data.lefty , 0, 255, 0, 180);
  data.rightx = map(data.rightx, 0, 255, 0, 180);
  data.righty = map(data.righty, 0, 255, 0, 180);
}

//my 40A ESC apparently cannot flash firmware, so I'm set with the default pwm ranges,
//which through testing is 40-170 in the context of the servo library
int mtrTrim(byte val) {
  if (val < 40) {
    return 40;
  } else if (val <= 170) {
    return val;
  } else if (val <= 180) {
    return 170;
  } else {
    return 40;
  }
}






// #include <Servo.h>
// #include<Wire.h>

// #define slaveAddress 0x08
// byte dataArray[6] = {90, 90, 90, 90, 90, 90};

// // dataWire[0] = data.leftx ;
// // dataWire[1] = data.lefty ;
// // dataWire[2] = (byte)data.leftb ;
// // dataWire[3] = data.rightx ;
// // dataWire[4] = data.righty ;
// // dataWire[5] = (byte)data.rightb ;


// Servo mtr; //throttle
// Servo rudd; //rudder
// Servo elev; //elevator
// Servo lail; //left aileron
// Servo rail; //right aileron

// void setup() {
//   // put your setup code here, to run once:
//   mtr.attach(3);
//   rudd.attach(5);
//   elev.attach(6);
//   lail.attach(9);
//   rail.attach(10);

//   Wire.begin(slaveAddress);
//   Serial.begin(9600);
//   Wire.onReceive(receiveEvent);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   mtr.write(mtrTrim(dataArray[1]));
//   rudd.write(dataArray[0]);
//   elev.write(180-dataArray[4]);
//   lail.write(dataArray[3]);
//   rail.write(180-dataArray[3]);
// }

// void receiveEvent(int howmany) //howmany = Wire.write()executed by Master
// {
//   for(int i=0; i<howmany; i++)
//   {
//     dataArray[i] = Wire.read();
//   }
// }

// float mtrTrim(byte val) {
//   if (val < 40) {
//     return 0;
//   } else if (val <= 140) {
//     return map(val, 40, 140, 0, 180);
//   } else if (val <= 180) {
//     return 180;
//   } else {
//     return 0;
//   }
// }