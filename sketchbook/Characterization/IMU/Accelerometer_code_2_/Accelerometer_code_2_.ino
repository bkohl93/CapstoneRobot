int scale = 200; //range for the adxl 377 is +-200g
boolean micro_is_5V = true; //set false if Vdd is not 5V
float scaledX, scaledY, scaledZ; // Scaled values for each axis

void setup()
{
  Serial.begin(9600);
}

double mapf(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop()
{
  // Get raw accelerometer data for each axis
  int rawX = analogRead(A0);
  int rawY = analogRead(A1);
  int rawZ = analogRead(A2);
  
   if (micro_is_5V) // microcontroller runs off 5V
    {
      scaledX = mapf(rawX, 0.0, 675, -scale, scale); // Mapf is Map() but not just for integers. 3.3/5 * 1023 =~ 675
    }
    
  else // microcontroller runs off 3.3V
    {
      scaledX = mapf(rawX, 0.0, 1023, -scale, scale);
    }
  
  Serial.print("X: "); Serial.println(rawX);   // Print out raw X,Y,Z accelerometer readings

  Serial.print("X: "); Serial.print(scaledX); Serial.println(" g");     // Print out scaled X,Y,Z accelerometer readings
  
  delay(2000);
  
 } 

