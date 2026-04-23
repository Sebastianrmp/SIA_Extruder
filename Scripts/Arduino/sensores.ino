//:::::termistor:::::::
float ntc() {
  termistorRes = ((float)analogRead (termistorPin)* VoltageDividerResistor)/(1023 - (float)analogRead (termistorPin));
  steinhart = termistorRes / termistorNominalRes;     // (R/Ro)
  steinhart = log(steinhart);                         // ln(R/Ro)
  steinhart /= termistorBValue;                       // 1/B * ln(R/Ro)
  steinhart += 1.0 / (termistorNominalTemp + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                        // invertir 
  steinhart -= 273.15;                                // pasar a C
  return steinhart;
} 
//:::::control de temperatura:::::::
void pid(int temp)
{
   // First we read the real value of temperature
  temperature_read = temp;
  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temp - temperature_read;
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-3
  if(-3 < PID_error <3)
  {
    PID_i = PID_i + (ki * PID_error);
  }

  //For derivative we need real time to calculate speed change rate
  timePrev = Time;                            // the previous time is stored before the actual time read
  Time = millis();                            // actual time read
  elapsedTime = (Time - timePrev) / 1000; 
  //Now we can calculate the D calue
  PID_d = kd*((PID_error - previous_error)/elapsedTime);
  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  //We define PWM range between 0 and 255
  if(PID_value < 0)
  {    PID_value = 0;    }
  if(PID_value > 255)  
  {    PID_value = 255;  }
  //Now we can write the PWM signal to the mosfet on digital pin D3
  analogWrite(PWM_pin,255-PID_value);
  previous_error = PID_error;     //Remember to store the previous error for next loop.

  delay(300);
  }

  
