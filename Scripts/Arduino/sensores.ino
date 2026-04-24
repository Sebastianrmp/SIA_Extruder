//:::::termistor:::::::
float ntc() {
  //Lectura analogica termistor
  termistorRes = ((float)analogRead (termistorPin)* VoltageDividerResistor)/(1023 - (float)analogRead (termistorPin));
  //Ecuación Steinhart-Hart
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
   //Primero se lee el valor real de temperatura
  temperature_read = temp;
  //Luego calculamos el error entre el punto de ajuste y el valor real
  PID_error = set_temp - temperature_read;
  //Cálculo de P
  PID_p = kp * PID_error;
  //Cálculo de I en un rango de +-3
  if(-3 < PID_error <3)
  {
    PID_i = PID_i + (ki * PID_error);
  }

  //Para derivativo necesitamos tiempo real para calcular la taza de cambio de velocidad
  timePrev = Time;                            // El tiempo anterior se almacena antes de la lectura actual
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

  
