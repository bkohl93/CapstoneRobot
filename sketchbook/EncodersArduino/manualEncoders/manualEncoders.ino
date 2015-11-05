/* This code is meant to be used to characterize the robot.
   The control functions are included from GoToPoint, so put
   commands into void loop(){...} to make it do stuff */

#include <ros.h>
#include <std_msgs/Int32.h>


// Set up ROS publishing
ros::NodeHandle  nh;
std_msgs::Int32 leftencoder_msg;
std_msgs::Int32 rightencoder_msg;
ros::Publisher pub_LeftEncoder("LeftEncoder", &leftencoder_msg);
ros::Publisher pub_RightEncoder("RightEncoder", &rightencoder_msg);

// Static constants
static int pulseRatio = 600; // Number of encoder counts per rotation (or 1200?)
static double wheelDiameter = 7.75; // Diameter of the wheels in inches
static double wheelBase = 38; // Distance between wheels in inches

// Global Variables
long long countR, countL; // Encoder count
bool stateR, stateL, lastStateR, lastStateL, left, right;

void setup()
{
  nh.initNode();  
  nh.advertise(pub_LeftEncoder);
}

void loop () {
  if (analogRead(A2 ) > 500)
    {
      stateL = 1;
    }
    else
    {
      stateL = 0;
    }
    if (lastStateL != stateL)
    {
      countL++;
    }
    lastStateL = stateL;
    if (countL % 10 == 0) {
       leftencoder_msg.data = countL; 
       pub_LeftEncoder.publish( &leftencoder_msg);
       nh.spinOnce();
    }
}

