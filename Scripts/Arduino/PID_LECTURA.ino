/*    Max6675 Module  ==>   Arduino
 *    CS              ==>     D10
 *    SO              ==>     D12
 *    SCK             ==>     D13
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */
#include <SPI.h>
#include <math.h>


float rAux = 102600;
float vcc = 5;
float beta = 3977.0;
float temp0 = 298.0;
float r0 = 88600;

float vm = 0;
float rntc = 0;
float tK = 0;
//LCD config

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f,20,4);  //sometimes the adress is not 0x3f. Change to 0x27 if it dosn't work.

/*    i2c LCD Module  ==>   Arduino
 *    SCL             ==>     A5
 *    SDA             ==>     A4
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */

//I/O
int PWM_pin = 3;  //Pin for PWM signal to the MOSFET driver (the BJT npn with pullup)
int clk = 8;      //Pin 1 from rotary encoder
int data = 9;     //Pin 2 from rotary encoder



//Variables
float set_temperature = 50;            //Default temperature setpoint. Leave it 0 and control it with rotary encoder

float temperature_read = 0.0;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
float PID_value = 0;
int button_pressed = 0;
int menu_activated=0;
float last_set_temperature = 0;

//Vraiables for rotary encoder state detection
int clk_State;
int Last_State;  
bool dt_State;  

//PID constants
//////////////////////////////////////////////////////////
int kp = 90;   int ki = 30;   int kd = 80;
//////////////////////////////////////////////////////////

int PID_p = 0;    int PID_i = 0;    int PID_d = 0;
float last_kp = 0;
float last_ki = 0;
float last_kd = 0;
int PID_values_fixed =0;

void setup() {
  Serial.begin(9600);
  pinMode(PWM_pin,OUTPUT);
  Time = millis();
 
  lcd.init();
  lcd.backlight();
}

void loop() {
//if(menu_activated==0)
//{
  // First we read the real value of temperature
  temperature_read = readThermocouple();
  lcd.setCursor(0,0);
  lcd.print("Temperature: ");
  lcd.setCursor(0, 14);
  lcd.print(temperature_read);
  lcd.setCursor(1,0);
  lcd.println(" degrees Celsius.");
  
  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temperature - temperature_read + 3;
  //Calculate the P value
  PID_p = 0.01*kp * PID_error;
  //Calculate the I value in a range on +-3
  PID_i = 0.01*PID_i + (ki * PID_error);
  

  //For derivative we need real time to calculate speed change rate
  timePrev = Time;                            // the previous time is stored before the actual time read
  Time = millis();                            // actual time read
  elapsedTime = (Time - timePrev) / 1000; 
  //Now we can calculate the D calue
  PID_d = 0.01*kd*((PID_error - previous_error)/elapsedTime);
  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  //We define PWM range between 0 and 255
  if(PID_value < 0)
  {    PID_value = 0;    }
  if(PID_value > 255)  
  {    PID_value = 255;  }
  //Now we can write the PWM signal to the mosfet on digital pin D3
  //Since we activate the MOSFET with a 0 to the base of the BJT, we write 255-PID value (inverted)
  analogWrite(PWM_pin,255-PID_value);
  previous_error = PID_error;     //Remember to store the previous error for next loop.

  float end_time = millis();

  delay(250 - (Time - end_time)); //Refresh rate + delay of LCD print
  //Antirreloj
}


//The function that reads the SPI data from MAX6675
double readThermocouple() {

  long sum = 0;
  const int numSamples = 10;
  for(int i = 0; i < numSamples; i++) {
    sum += analogRead(A0);
    delay(10);
  }
  int adcValue = sum / numSamples;


  vm = (vcc/1024)*(adcValue);
  rntc = rAux/((vcc/vm)-1);
  tK = beta/(log(rntc/r0)+(beta/temp0));
  return tK-273;
}










