#include <ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Int32.h>
#include <ArduinoHardware.h>

#include <RedBot.h>
//#include <RedBotSoftwareSerial.h>


// Flag for bumpers to send the signal that something's wrong and the motors should
//  be disabled for now. Set in the bump functions, volatile so the change is sure
//  to propagate to the loop() function.
volatile boolean bumped = true;


ros::NodeHandle nh;


// Create an alias for the onboard pushbutton.
#define BUTTON_PIN 12

// Create an alias for the onboard LED.
#define LED_PIN 13


///////////////
// Motor

// Instantiate the motors.
RedBotMotor motors;


void motorCB( const std_msgs::Bool& motorOn){
  if (motorOn.data && !bumped)
  {
    motors.drive(200);

  }
  else motors.brake();
}

ros::Subscriber<std_msgs::Bool> sub("motor", &motorCB );


/////  Encoder
// Instantiate our encoder. 
RedBotEncoder encoder = RedBotEncoder(A2, A3); // left, right

std_msgs::Int32 encoder_msg;
ros::Publisher lwheel_pub("lwheel", &encoder_msg);
ros::Publisher rwheel_pub("rwheel", &encoder_msg);


// Instantiate a couple of whisker switches. Call bump() when one of them
//  hits something. There's no stopping you having a different function for
//  each whisker; I just chose not to.
RedBotBumper lBumper(10, &bump);
RedBotBumper rBumper(11, &bump);

std_msgs::Bool pushed_msg;
ros::Publisher bumper("bumper", &pushed_msg);


void setup() {
  nh.initNode();
  nh.advertise(bumper);
  nh.subscribe(sub);
  nh.advertise(lwheel_pub);
  nh.advertise(rwheel_pub);

  // Set up our two built-in IO devices- the button and the LED.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  bumped=false;

}

void loop() { 
  static unsigned long loopStart = millis();

  // Wait for the button to be pressed, then turn off the "bumped" flag
  //  so the motors can run. Also, clear the encoders, so we can track
  //  our motion.
  if (bumped == true && digitalRead(10)== HIGH && digitalRead(11) == HIGH)
  {
    bumped = false;
    pushed_msg.data = false;
    bumper.publish(&pushed_msg);

  } 

  // TODO(chalko) how often ?
  if(loopStart +  100 <  millis()) {
    loopStart = millis();
    encoder_msg.data = encoder.getTicks(LEFT);
    lwheel_pub.publish(&encoder_msg);
    encoder_msg.data = encoder.getTicks(RIGHT)+7;
    rwheel_pub.publish(&encoder_msg);
  }
  nh.spinOnce();
}

// This is the function that gets called when we bump something. It
//  stops the motors, signals that a bump occurred (so loop() doesn't
//  just start the motors back up), and issues a nasty little tone to
//  tell the user what's up.
void bump()
{
  motors.brake();
  pushed_msg.data = true;
  bumper.publish(&pushed_msg);
  bumped = true;
}


