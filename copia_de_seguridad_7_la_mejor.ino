#include <EEPROM.h>
#include <Wire.h>
#include <Servo.h>
#include "HX711.h" 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f, 20, 4);
Servo servoMotor;
HX711 balanza;
//parametros NTC
#define termistorPin A0
#define termistorNominalRes 100000
#define termistorNominalTemp 25
#define termistorBValue 3950
#define VoltageDividerResistor 100000
float termistorRes = 0.0; 
float steinhart;
int PWM_pin = 10;
int salida=0;
//Variables PID
float temperature_read = 0.0;
//float set_temperature = 0;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
int PID_value = 0;

//PID constantes
int kp = 9.1;   int ki = 0.3;   int kd = 1.8;
int PID_p = 0;    int PID_i = 0;    int PID_d = 0;
//Parametros Encoder
const int salida_A = 5;    //CLK 
const int salida_B = 6;    //DT
const int swPin  = 7;      //SW
//parametros celda de carga
const int DOUT=A2;
const int CLK=A1;
//variables
int set_abs=220;
int set_pet=240;
int set_pla=190;
int set_temp=0;
int set_pes=0;
int contador = 0; 
int new_temp = 180;
int new_pes =50;
int dob=0;
int A_estado_actual;
int A_ultimo_estado;
int velocidad_scroll = 300;
int servoac=0;

//::::::::::MENU::::::::::::::::::::::
String menu0[] = {"Pulsa el boton","para continuar","",""};

String menu1[] = {" ","establecer datos","extrusion continua"," "};
int sizemenu1 = sizeof(menu1) / sizeof(menu1[0]); 

String menu2[] = {"sel temperatura","sel peso","continuar","atras"};  //Inicializamos nuestro Array con los elementos del menu
int sizemenu2 = sizeof(menu2) / sizeof(menu2[0]);                           //Obtenemos el número de elementos ocupados en la matriz. en este caso 6     

String menu3[] = {"Abs","Pet","Pla","Atras"};
int sizemenu3 = sizeof(menu3) / sizeof(menu3[0]);

String menu4[] = {"  pulsa ","para establecer ","T:    C"};
int sizemenu4 = sizeof(menu4) / sizeof(menu4[0]);

String menu5[] = {"  pulsa ","para establecer ","W:    g"};
int sizemenu5 = sizeof(menu5) / sizeof(menu5[0]);

String menu6[] = {"en operacion","pulsa para cancelar","T:    C  W:    g","T:    C  W:    g"};
int sizemenu6 = sizeof(menu6) / sizeof(menu6[0]);

String menu7[] = {"","Continuar","realizar cambios",""};
int sizemenu7 = sizeof(menu7) / sizeof(menu7[0]);

String linea1,linea2,linea3,linea4;           //Lineas del LCD
int seleccion = 0;
int pos = 0;
int level_menu = 0;             //Iniciamos la variable en el menu principal 0 --> 6
int opcion = 0;
bool btnpress = false;          //Esta variable de retorno en false aegurando que el boton del Encoder aun no se ha oprimido
byte flecha[] = {B10000,B11000,B11100,B11110,B11100,B11000,B10000,B00000};      //Creamos un array de 8 posiciones para la flecha del menu
byte aa[] = {B00011,B00011,B01011,B11011,B11011,B11011,B11011,B11011};  
byte ab[] = {B01110,B01110,B01110,B01110,B01110,B01110,B01110,B01110};
byte ac[] = {B11100,B11110,B11111,B11111,B11011,B11011,B11000,B11111};
byte ad[] = {B11011,B11011,B11011,B11011,B11011,B11111,B01111,B00111};
byte ae[] = {B11111,B00011,B11011,B11011,B11011,B11111,B11110,B11100};

void setup() {
  Time = millis(); 
  Serial.begin(9600);                     //Habilitamos la salida serial por USB
  pinMode(PWM_pin,OUTPUT);
  TCCR1B = TCCR1B & B11111000 | B00000011;
  
  servoMotor.attach(11);
  servoMotor.write(25);
  delay(500);
  servoMotor.write(55);
  delay(500);
  
  pinMode (salida_A,INPUT);
  pinMode (salida_B,INPUT);
  pinMode (swPin,INPUT_PULLUP);
  //Caracteres personalizados 
  lcd.init();
  lcd.backlight();
  lcd.createChar (0,aa);
  lcd.createChar (1,ab);
  lcd.createChar (2,ac);
  lcd.createChar (3,ad);
  lcd.createChar (4,ae);
  lcd.createChar(5, flecha);
                      
  balanza.begin(DOUT, CLK);
  balanza.set_scale(439430.25); // Establecemos la escala
  balanza.tare(20);  //El peso actual es considerado Tara.
  
  A_ultimo_estado = digitalRead(salida_A);         //Leemos el estado de la salida del Encoder usando el pin CLK
  fn_creditos();
  fn_menu(contador,menu0,1);
}

void loop() {
selectOption();
ntc();
salida=ntc(); 
 //:::::: menu de la pantalla 0:::
if(level_menu == 0){
  lcd.clear();
  lcd.setCursor(2,2);
  lcd.print(salida); 
  fn_menu(contador,menu0,1);
  delay(200);   
  if(btnpress){
    contador = 0;
    fn_menu(contador,menu1,2); 
    level_menu=1;
    }
    
    btnpress = false;
 }  
//:::::: menu de la pantalla 1:::                           
if(level_menu == 1){                       
 
    if(fn_encoder(2) ){               
      fn_menu(contador,menu1,2);     
    }
 
    if(btnpress){                             
      //Led 1      
      if(contador == 0){
          contador = 0;                       
          fn_menu(contador,menu2,sizemenu2);  
          level_menu = 2;                                       
      }
      if(contador == 1){
          contador = 0;                       
          fn_menu(contador,menu3,sizemenu3);  
          level_menu = 3;                                       
      }
      
    }
    btnpress = false;
    }
//:::::: menu de la pantalla 2 :::::  
if(level_menu == 2){
    if(fn_encoder(sizemenu2) ){               
      fn_menu(contador,menu2,sizemenu2);     
    }

        if(btnpress){                             
      //Led 1      
      if(contador == 0){
          contador = 0;                        
          level_menu = 4;                                       
      }
      if(contador == 1){
          contador = 0;                         
          level_menu = 5;                                       
      }
      if(contador == 2){
          contador = 0;                         
          level_menu = 6;                                       
      }
      if(contador == 3){
          contador = 0;
          fn_menu(contador,menu1,2);                         
          level_menu = 1;                                        
      }
      
    }
    btnpress = false;
  }
 //:::::: menu de la pantalla 3 :::::  
if(level_menu == 3){
    if(fn_encoder(sizemenu3) ){               
      fn_menu(contador,menu3,sizemenu3);     
    }

        if(btnpress){                             
      //Led 1      
      if(contador == 0){
          contador = 0;
          set_temp = set_abs;
          set_pes = 0;                         
          level_menu = 6;                                       
      }
      if(contador == 1){
          contador = 0;
          set_temp = set_pet;
          set_pes = 0;                         
          level_menu = 6;                                       
      }
      if(contador == 2){
          contador = 0;
          set_temp = set_pla;
          set_pes = 0;                          
          level_menu = 6;                                       
      }
      if(contador == 3){
          contador = 0;
          fn_menu(contador,menu1,2);                         
          level_menu = 1;                                       
      }
      
    }
    btnpress = false;
  }
  
//:::::: menu de la pantalla 4 :::::
 
  if(level_menu == 4){              
      lcd.clear();
      fn_encoder(350);
      lcd.setCursor(1,0);
      lcd.print(menu4[0]);
      lcd.setCursor(1,1);
      lcd.print(menu4[1]);
      lcd.setCursor(1,2);
      lcd.print(menu4[2]);
      lcd.setCursor(2,2);
      lcd.print(new_temp);  
      delay(200);
        if(btnpress){                   
          contador = 0;
          fn_menu(contador,menu2,sizemenu2); 
          set_temp=new_temp;                        
          level_menu = 2; 
          new_temp=180; 
          new_pes=50;       

            }
          btnpress = false;
          
  }
  
//:::::: menu de la pantalla 5 :::::
 
  if(level_menu == 5){              
      lcd.clear();
      fn_encoder(100);
      lcd.setCursor(1,0);
      lcd.print(menu5[0]);
      lcd.setCursor(1,1);
      lcd.print(menu5[1]);
      lcd.setCursor(1,2);
      lcd.print(menu5[2]);
      lcd.setCursor(2,2);
      lcd.print(new_pes);  
      delay(200);
        if(btnpress){                   
          contador = 0;
          fn_menu(contador,menu2,sizemenu2); 
          set_pes=new_pes;                        
          level_menu = 2;
          new_pes=50;
          new_temp=180;         

            }
          btnpress = false;
          
  }

    
   //:::::: menu de la pantalla 6 :::::
 
  if(level_menu == 6){
      pid(salida);                  
      lcd.clear();
      fn_menu(contador,menu6,1);
      lcd.setCursor(3,2);
      lcd.print(set_temp); 
      lcd.setCursor(13,2);
      lcd.print(set_pes); 
      lcd.setCursor(3,3);
      lcd.print(salida);
          
      delay(200);
        if(btnpress){contador = 0;
          fn_menu(contador,menu1,2);                         
          level_menu = 1;
          set_temp=0;
          }
          btnpress = false;
          
  }
}

void selectOption(){
  if(digitalRead(swPin) == LOW){
    delay(500);
    btnpress = true;
  }
}

void fn_creditos(){
 lcd.setCursor(1, 0);
 lcd.write(0);
 lcd.write(1);
 lcd.write(2);
 lcd.setCursor(1, 1);
 lcd.write(3);
 lcd.write(1);
 lcd.write(4);
 lcd.setCursor(5,0);
 lcd.print("Extrusora de");
 lcd.setCursor(7,1);
 lcd.print("Filameto ");
 lcd.setCursor(9,2);
 lcd.print("3D SIA");
 lcd.setCursor(0,3);
 lcd.print("V:001");
 delay (3000);
 //level_menu = 1;
 lcd.clear();
}


void fn_menu(int pos,String menus[],byte sizemenu){
  lcd.clear();
  linea1 = menus[0];
  linea2 = menus[1];
  linea3 = menus[2];
  linea4 = menus[3];
  if (sizemenu ==1 ){lcd.setCursor(0, 1);lcd.print("");}
  if (sizemenu ==2 ){
    if(pos == 0){ 
     lcd.setCursor(0, 1);
     lcd.write(byte(5));}
   if(pos == 1){ 
     lcd.setCursor(0, 2);
     lcd.write(byte(5));}
    }
  if (sizemenu > 2 ){
  if(pos == 0){ 
     lcd.setCursor(0, 0);
     lcd.write(byte(5));}
   if(pos == 1){ 
     lcd.setCursor(0, 1);
     lcd.write(byte(5));}  
    if(pos == 2){ 
     lcd.setCursor(0, 2);
     lcd.write(byte(5));}
    if(pos == 3){ 
     lcd.setCursor(0, 3);
     lcd.write(byte(5));} 
    }
     lcd.setCursor(1, 0);
     lcd.print(linea1);

     lcd.setCursor(1, 1);
     lcd.print(linea2);
     
     lcd.setCursor(1, 2);
     lcd.print(linea3);
     
     lcd.setCursor(1, 3);
     lcd.print(linea4); 
    
   
}


bool fn_encoder(byte sizemenu){ 
  bool retorno = false;

  A_estado_actual = digitalRead(salida_A); 
  
  if (A_estado_actual != A_ultimo_estado){     

    if (digitalRead(salida_B) != A_estado_actual){        //DT != CLK     
     contador ++;
     new_temp ++;
     new_pes ++;
     delay(250);     
    }
    else {
     contador --;
     new_temp --;
     new_pes --;
     delay(250); 
    }
    
    if(contador <=0){
      contador = 0;
    }
    
    if(contador >= sizemenu-1 ){
      contador = sizemenu-1;
    }
    
    retorno = true;

  } 

  return retorno; 
}
