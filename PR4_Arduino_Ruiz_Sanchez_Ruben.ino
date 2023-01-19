



//--------------------[GENERAL]----------------------//
#include <Arduino.h>
#include <Wire.h>
//--------------------[KEYPAD/BOTONERA]----------------------//
#include "I2CKeyPad.h"
const uint8_t KEYPAD_ADDRESS = 0x20; //Direcció modul expansor pcF8574
I2CKeyPad keyPad(KEYPAD_ADDRESS); //Llibreria
uint32_t start, stop;
uint32_t temps_lletra = 0;


//--------------------[Sensor Moviment/PIR]----------------------//
int sensor_mov = 2;              // El pin al qual està connectat el sensor
int estat_mov = LOW;         // Per defecte, no s'ha detectat cap moviment
int moviment = 0;            //Variable per emmagatzemar l'estat del sensor


//--------------------[LCD MONITO]----------------------//

#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int mostrar = 99;

//--------------------[RELE]----------------------//
//  int rele = 4; 

//--------------------[So]----------------------//
//Definim So del Buzzer/Altaveu i asignem freqüència
#define S_Do 261 
//Declarem variables i ennllaçem amb la poscició buzzer
const int Buzzer = 3;
//--------------------[Servo]--------------------//
#include <Servo.h> //Importem llibreria pel Servo
// Declarem la variable per a controlar el servo
Servo pany;
//--------------[Structura de control]-----------//
//Declarem STRUCT de control
struct control {
	int  inici;//controla inici programa
	int  pos_servo;//posició del servo
	int  porta;//status de laporta 0 oberta , 1 tancada
	int  alarma;//status de la alarma 0 desactivada, 1 activada
	int  gas;//status sensor gas 0 desactivada, 1 activada
	int  aigua;//status sensor gas 0 desactivada, 1 activada
	int  moviment;//status sensor moviment 0 desactivat, 1 activada
	int  contrasenya;//contrasenya 
};//struct
typedef control Control;
Control status;
//-------------------[Botó Opertura interna]---------------------//
//Declarem variables i enllaçem amb la poscició dels botons
const int Boto_obrir = 13;
//-------------------[Detector Gas]---------------------//
//Declarem variables i enllaçem amb la poscició dels botons
#define Sensor_Gas A0
//-------------------[Detector Aigua]---------------------//
//Declarem variables i enllaçem amb la poscició dels botons
#define Sensor_Aigua A1


void setup() {
	Serial.begin(115200);
	
	//--------------------[Servo]----------------------//
	// Iniciem el servo perquè comenci a treballar amb el pin 9
	pany.attach(9, 500, 2500);
	
	//--------------------[LCD]----------------------//
	int error;//	
	Wire.begin();
	Wire.beginTransmission(0x27); //Transmissió al monitor LCD
	error = Wire.endTransmission();
	Serial.print("Error: ");
	Serial.print(error);
	
	if (error == 0) {
		Serial.println("\n OK: LCD");
		mostrar = 0;
		lcd.begin(20, 4);  //Initcialitzem LCD
		} else {
		Serial.println("\n ERROR: LCD");
	}  // if control monitor
	
	//--------------------[KEYPAD/BOTONERA]----------------------//
	if (keyPad.begin() == false)  {
		Serial.println("\n ERROR: Keypad");
		}else {
		Serial.println("\n OK: Keypad");
	}//if control Keypad
	Wire.setClock(400000);
	
	//--------------------[Sensor Moviment/PIR]----------------------//
	pinMode(sensor_mov, INPUT);    // initialize sensor as an input
	
	
	//--------------------[RELE]----------------------//
	//  pinMode(rele, OUTPUT);
	
	//--------------------[Servo]----------------------//
	// Iniciem el servo perquè comenci a treballar amb el pin 9
	//	pany.attach(8, 500, 2500);
	pany.attach(8, 500, 2500);
	
	//--------------------[BOTONS]----------------------//
	//Definim pins/botons com a entrada i els inicialitzem	
	pinMode(Boto_obrir, INPUT); 
	digitalWrite(Boto_obrir,HIGH);
	
	//--------------------[STATUS]----------------------//
	// Iniciem variable senyal de control 
	status.inici = 0;
	status.pos_servo = 90;
	status.alarma = 1;
	status.porta = 1;
	status.gas = 0;
	status.aigua = 0;
	status.moviment = 0;  
	status.contrasenya =  1990;
	
	//-------------------[Detector Aigua]---------------------//
	pinMode(Sensor_Aigua, INPUT);
	
	
}  // setup()



int Detectar_Gas(struct control *temp_status) {
	unsigned int sensor_valor = analogRead(Sensor_Gas);  // Llegeix el valor analògic del sensor
	unsigned int sortida_valor = map(sensor_valor, 0, 1023, 0, 255); // Mapeja les dades de 10 bits a dades de 8 bits
	if (sortida_valor > 100)
	temp_status->gas = 1;
	//else
	//temp_status->gas = 0;
	return sortida_valor;             //Retorna el valor analògic
}//funció Detectar_Gas

int Detectar_Aigua(struct control *temp_status) {
	unsigned int sensor_valor = analogRead(Sensor_Aigua);  // Llegeix el valor analògic del sensorunsigned int sortida_valor = map(sensor_valor, 0, 1023, 0, 255); // Mapeja les dades de 10 bits a dades de 8 bits
	if (sensor_valor < 600)
	temp_status->aigua = 1;
//	else
	//temp_status->aigua = 0;
	return sensor_valor;             //Retorna el valor analògic
}//funció Detectar_Gas

void Detectar_moviment(struct control *temp_status) {
	int alarma = temp_status->alarma;
	moviment = digitalRead(sensor_mov);   // llegeix el valor del sensor
	if (moviment == HIGH) {           // comproveu si el sensor és actiu
		delay(100);                // retard 100 mil·lisegons
    	Serial.println("\n  Moviment!"); //Informació en serie 
		if (estat_mov == LOW && alarma == 1) {
			Serial.println("\n S'ha detectat moviment amb alarma activa!"); //Informació en serie 
			estat_mov = HIGH;       // actualitza l'estat de la variable a actiu
			temp_status->moviment = 1;//Actualitzem variable general
		}
		}/*else {
		delay(200);             // retard 200 mil·lisegons
		if (estat_mov == HIGH){
			Serial.println("\n Ja no es detecta moviment"); //Informació en serie 
			estat_mov = LOW;       // actualitza l'estat de la variable a inactiu
			temp_status->moviment = 0;//Actualitzem variable general
		}
	}// if detecta moviment
  */
}//funció int Detectar_moviment


int ficar_contrasenya(struct control *temp_status) {
	
	
	char lletra = 'x';
	int num = 0;
	int resultat = 0;
	int max_lletres = 4;
	int actual_lletres = 1;
	lcd.setBacklight(255);//activem llum
	lcd.setCursor(1,1);
	lcd.print("       [****]");//Imprimim char per el lcd 
	delay(1000);
	lcd.clear();
	while (lletra != '#'){
		uint32_t ara_lletra = millis();
		bool keypad_conectat  = keyPad.isConnected();
		bool keypad_polsat  = keyPad.isPressed();
		uint8_t ultima_lletra = keyPad.getLastKey();
		uint8_t index = keyPad.getKey();
		
		if (ara_lletra - temps_lletra >= 100)  {
			if( keypad_polsat == true){
				lletra = ("123A456B789C*0#DNF"[index]);
				temps_lletra = ara_lletra;
				
				if(lletra >= '0' && lletra <= '9'){
					num = (int) lletra - 48;//pasem char ascii a sencer
					resultat = num  + (resultat * 10);
				}// si es un num
				
				Serial.print("Num ");
				Serial.print(num);
				
				if(lletra != 'D' && lletra != '#'){
					switch (actual_lletres) {
						case 1:
						lcd.print("       [");//Imprimim char per el lcd
						lcd.print(lletra);//Imprimim char per el lcd  
						lcd.print("***]");//Imprimim char per el lcd             
						actual_lletres++;
						break;
						case 2:
						lcd.print("       [*");//Imprimim char per el lcd
						lcd.print(lletra);//Imprimim char per el lcd  
						lcd.print("**]");//Imprimim char per el lcd 
						actual_lletres++;
						break;
						case 3:
						lcd.print("       [**");//Imprimim char per el lcd
						lcd.print(lletra);//Imprimim char per el lcd  
						lcd.print("*]");//Imprimim char per el lcd 
						actual_lletres++;
						break;
						case 4:
						lcd.print("       [***");//Imprimim char per el lcd
						lcd.print(lletra);//Imprimim char per el lcd  
						lcd.print("]");//Imprimim char per el lcd
						actual_lletres++;
						break;
					}//switch
					delay(1000);
					lcd.clear();
					if (actual_lletres >= 5){actual_lletres=4;}//IF actual_lletres > 4
				}//If control lletres D  #
				
				if(lletra == '#'){
					break;
				}//iF ACCEPTAR
				if(lletra == 'D'){
					actual_lletres--;
					if (actual_lletres <= 0){actual_lletres=1;}//IF actual_lletres < 1
				}//iF Esborrar
		}// Id boto polsat
	}//IF control polsacions
	
}//WHILE no es premi ok (#) estigui en bucle
return resultat;
}//funció ficar_contrasenya

void *des_bloqueig(struct control *temp_status, int bloqueig){
  Serial.println(" entra des_bloqueig");
	int intents = 3; //máxim número intents
	bool correcte = false;
	int contrasenya = 0;
	while (intents > 0 && correcte == false)
	{
		lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
		lcd.print("    DESBLOQUEIG!");//Imprimim string per el lcd
		lcd.setCursor(0,1); //Posem el cursor en la 2 fila a l'esquerra
		lcd.print("--------------------");//Imprimim string per el lcd 
		lcd.setCursor(0,2); //Posem el cursor en la 3 fila a l'esquerra
		lcd.print(" #=OK || D=esborrar");//Imprimim string per el lcd 
		lcd.setCursor(0,3); //Posem el cursor en la 4 fila a l'esquerra   
		lcd.print("--------------------");//Imprimim string per el lcd     
		delay(3000); //Esperem 3 segons
		
		lcd.clear(); //Esborrem  
		lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
		lcd.print("   Introduir codi");//Imprimim string per el lcd
		contrasenya = ficar_contrasenya(&status);
		delay(100);
		
		Serial.println("Contrasenya: "); //Informació en serie 
		Serial.println(contrasenya); //Informació en serie 
		
		if (contrasenya == status.contrasenya){
			lcd.clear(); //Esborrem  
			lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra			
			lcd.print(" Contrasenya");//Imprimim string per el lcd
			lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
			lcd.print("    Correcte");//Imprimim string per el lcd
			delay(3000); //Esperem 3 segons   //// ATENCIO  ATENCIO ATENCIO        				
			lcd.clear(); //Esborrem  
			correcte = true;
			break;
			}else{
			intents--;
			if(intents == 0){
				lcd.clear(); //Esborrem       
				lcd.setCursor(0,0); //Posem el cursor en la 1 fila a l'esquerra
				lcd.print("     CONTRASENYA");//Imprimim string per el lcd
				lcd.setCursor(0,1); //Posem el cursor en la 2 fila a l'esquerra
				lcd.print("      INCORRECTE");//Imprimim string per el lcd
				lcd.setCursor(0,2); //Posem el cursor en la 3 fila a l'esquerra
				lcd.print("--------------------");//Imprimim string per el lcd
				lcd.setCursor(0,3); //Posem el cursor en la 3 fila a l'esquerra
				lcd.print("  Intents esgotats");//Imprimim string per el lcd
				delay(3000); //Esperem 3 segons		
				alarma_so();//Activem so
				break;              
			};//Error al introduir contrasenya
			
			lcd.clear(); //Esborrem       
			lcd.setCursor(0,0); //Posem el cursor en la 1 fila a l'esquerra
			lcd.print("     CONTRASENYA");//Imprimim string per el lcd
			lcd.setCursor(0,1); //Posem el cursor en la 2 fila a l'esquerra
			lcd.print("      CORRECTE");//Imprimim string per el lcd
			lcd.setCursor(0,2); //Posem el cursor en la 3 fila a l'esquerra
			lcd.print("--------------------");//Imprimim string per el lcd
			lcd.setCursor(0,3); //Posem el cursor en la 3 fila a l'esquerra
			lcd.print("Intents restants: ");//Imprimim string per el lcd
			lcd.print(intents);//Imprimim string per el lcd
		}//If  contrasenya correcte
		delay(3000); //Esperem 3 segons
		lcd.clear(); //Esborrem  
	}//while 
	
	if(correcte == true){
		if(bloqueig == 0){
			obrir_pany(&status); 	
			}else{
			tancar_pany(&status); 	
		}//comprobem si Bloquegem o desbloquegem
		//llamar a funcion de sonido como de final fantasy y que parpade leden verde//-------------------------//
	}//if tots els intents esgotats
	lcd.clear(); //Esborrem 
}//funció desbloqueig

void *obrir_pany(struct control *temp_status){  
	if (temp_status->porta == 1){
		Serial.println("\n Obrint porta"); //Informació en serie 
		float pos = temp_status->pos_servo;
		float percentatge = 0;
		delay(1000);
		while (pos <=270){
			pany.write(pos);
			pos++;
			lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
			lcd.print("  Obrint Porta");//Imprimim string per el lcd
			lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
			percentatge = (pos/270)*100;
			if (percentatge < 10){
				lcd.print("       0");
				}else{
				lcd.print("       ");
			}// if percentatge <10
			lcd.print((int)percentatge);
			lcd.print("%");
			delay(100);
			lcd.clear(); //Esborrem      
		}//while servo
		temp_status->pos_servo = 270;
		lcd.clear(); //Esborrem
		}else{
		lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
		lcd.print("     ERROR!");//Imprimim string per el lcd
		lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
		lcd.print("Porta ja oberta");//Imprimim string per el lcd
		delay(3000); //Esperem 3 segons
		lcd.clear(); //Esborrem      
	}//if(porta == 0)  
	temp_status->porta = 0;//indiquem porta Oberta (0)
	temp_status->alarma = 0;//indiquem alarma 
}//funció obrir_pany


void *tancar_pany(struct control *temp_status)
{  
	if (temp_status->porta == 0){
		Serial.println("\n Tancant porta"); //Informació en serie 
		float pos = temp_status->pos_servo;
		float percentatge = 0;
		float num = 0;
		delay(1000);
		while (pos >= 90){
			pany.write(pos);
			pos--;
			num++;
			lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
			lcd.print("  Tancant Porta");//Imprimim string per el lcd
			lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
			percentatge = (num/270)*100;
			if (percentatge < 10){
				lcd.print("       0");
				}else{
				lcd.print("       ");
			}// if percentatge <10
			lcd.print((int)percentatge);
			lcd.print("%");
			delay(100);
			lcd.clear(); //Esborrem      
		}//while servo
		temp_status->pos_servo = 90;
		lcd.clear(); //Esborrem
		}else{
		lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
		lcd.print("     ERROR!");//Imprimim string per el lcd
		lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
		lcd.print("Porta ja tancada");//Imprimim string per el lcd
		delay(3000); //Esperem 3 segons
		lcd.clear(); //Esborrem      
	}//if(porta == 0)  
	temp_status->porta = 1;//indiquem porta tancada _(1)
	temp_status->alarma = 1;//indiquem alarma 
}//funció tancar_pany


void *obrir_tancar_dins(struct control *temp_status){ 
	int porta =	temp_status->porta;//indiquem porta tancada _(1) 
	Serial.println("\n Gestió porta des de dins"); //Informació en serie 
	if ( digitalRead(Boto_obrir)  ==LOW ) {
		if (porta == 1){  
			obrir_pany(&status); 	
			}else{
			tancar_pany(&status); 	
		}//Si la porta ja está tancada obrim o viceversa
		delay(100);
	}// Si es prem el botó
}//funció obrir_dins // Obrir la porta desde dins de casa.


void *Detectar_tot(struct control *temp_status){ 
	int porta =	temp_status->porta;//
	int aigua =	temp_status->aigua;//
	int gas =	temp_status->gas;//
	int moviment =	temp_status->moviment;//
	int alarma =	temp_status->alarma;//
	
	Serial.println("\n Cridem a les funcions de detecció"); //Informació en serie 
	Detectar_Gas(&status);
	Detectar_Aigua(&status);
	Detectar_moviment(&status);
  delay(100); //Esperem 3 segons
  //------------------ Impresio en Serie control------------------------------//
  Serial.print( "porta "); //Informació en serie 
  	Serial.print( porta); //Informació en serie 
  Serial.print("\t");
  	Serial.print( "moviment "); //Informació en serie 
  	Serial.print( moviment); //Informació en serie 
  Serial.print("\t");
     Serial.print( "aigua "); //Informació en serie 
    Serial.print( aigua); //Informació en serie 
  Serial.print("\t"); 
  Serial.print( "gas "); //Informació en serie 
    Serial.print( gas); //Informació en serie 
    Serial.print("\t");
       Serial.print( "alarma "); //Informació en serie 
     Serial.print( alarma); //Informació en serie 
      //---------------------------------------------------------------------//
	if (aigua == 1 || gas == 1 || (moviment == 1 && alarma == 1)){
		lcd.clear(); //Esborrem   
		lcd.setBacklight(255);//Activem llum LCD
		lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
		lcd.print("       ALARMA");//Imprimim string per el lcd
		lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
		lcd.print("--------------------");//Imprimim string per el lcd
		lcd.setCursor(0,2); //Posem el cursor en la segona fila a l'esquerra
		lcd.print("  Incident detectat");//Imprimim string per el lcd
    lcd.setCursor(0,3); //Posem el cursor en la segona fila a l'esquerra
		lcd.print("--------------------");//Imprimim string per el lcd
		delay(3000);
		lcd.clear();
	}//IF INCIDENT
	
	//----------------------------------------------//
  lcd.clear();
	lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
	lcd.print("Porta   :");//Imprimim string per el lcd
	if (porta == 0){
		lcd.print("Oberta");//Imprimim string per el lcd
		}else{
		lcd.print("Tancada");//Imprimim string per el lcd
	}//if porta
	
	lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
	lcd.print("Moviment:");//Imprimim string per el lcd
	if (moviment == 0){
		lcd.print("No detectat");//Imprimim string per el lcd
		}else{
		lcd.print("Detectat");//Imprimim string per el lcd
	}//if porta
	
	lcd.setCursor(0,2); //Posem el cursor en la segona fila a l'esquerra
	lcd.print("Aigua   :");//Imprimim string per el lcd
	if (moviment == 0){
		lcd.print("No detectat");//Imprimim string per el lcd
		}else{
		lcd.print("Inundacio");//Imprimim string per el lcd
	}//if porta
	
	lcd.setCursor(0,3); //Posem el cursor en la segona fila a l'esquerra
	lcd.print("Gas     :");//Imprimim string per el lcd
	if (gas == 0){
		lcd.print("No detectat");//Imprimim string per el lcd
		}else{
		lcd.print("PERILL");//Imprimim string per el lcd
	}//if porta
	
	if (aigua == 1 || gas == 1 || (moviment == 1 && alarma == 1)){
		//alarma_so(); //DESACTIVEM PER EVITAR SOROLL
	}//if so
	
	delay(5000);
	lcd.clear(); //Esborrem 		
}//funció Detectar_tot /

void alarma_so(){
	float sin_val = 0;
	int to_val = 0;
	int i = 0;
	uint32_t ara_lletra = millis();
	uint32_t fi = ara_lletra + 10000;
	
	while (ara_lletra <= fi){
		sin_val = (sin(i*(3.1412/180)));
		to_val = 2000+(int(sin_val*1000));
		tone(Buzzer,to_val);
		delay(4);
		i++;
		if(i > 180){i = 0;}
		ara_lletra = millis();
	}//while 
} //alarma_so

void *menu(struct control *temp_status){
	bool selecio = false;
	char lletra = 'x';
	lcd.setCursor(0,0); //Posem el cursor en la primera fila a l'esquerra
	lcd.print("________MENU________");//Imprimim string per el lcd
	lcd.setCursor(0,1); //Posem el cursor en la segona fila a l'esquerra
	lcd.print("PORTA: OBRIR     [A]");//Imprimim string per el lcd
	lcd.setCursor(0,2); //Posem el cursor en la  3 a l'esquerra
	lcd.print("PORTA: TANCAR    [B]");//Imprimim string per el lcd
	lcd.setCursor(0,3); //Posem el cursor en la  4 a l'esquerra
	lcd.print("ESTAT: ALARMA    [C]");//Imprimim string per el lcd
	
	delay(9000); //Esperem 9 segons
	lcd.clear(); //Esborrem 
	
	
	while (selecio != true){
		uint32_t ara_lletra = millis();
		bool keypad_conectat  = keyPad.isConnected();
		bool keypad_polsat  = keyPad.isPressed();
		uint8_t ultima_lletra = keyPad.getLastKey();
		uint8_t index = keyPad.getKey();
		
		if (ara_lletra - temps_lletra >= 100)  {
			if( keypad_polsat == true){
				lletra = ("123A456B789C*0#DNF"[index]);
				temps_lletra = ara_lletra;
				
				switch (lletra) {
					case 'A':
					des_bloqueig(&status, 0); //0 per desbloquejar i 1 per bloquejar
					selecio = true;//control
					break;
					case 'B':
					des_bloqueig(&status, 1); //0 per desbloquejar i 1 per bloquejar
					selecio = true;//control
					break;
					case 'C':
					Detectar_tot(&status);
					selecio = true;//control
					break;					
				}//switch
			}// Id boto polsat
		}//IF control polsacions				
	}//while selection
	
}//funció menu	

void *cridar_menu(struct control *temp_status){
  	Serial.println("Cridar menu");
	int porta =	temp_status->porta;//
	int aigua =	temp_status->aigua;//
	int gas =	temp_status->gas;//
	int moviment =	temp_status->moviment;//
	int alarma =	temp_status->alarma;//
	
	uint32_t ara_lletra = millis();
	bool keypad_polsat  = keyPad.isPressed();
	uint8_t index = keyPad.getKey();
	char lletra = 'x';
	
	if (ara_lletra - temps_lletra >= 100)  {
		if( keypad_polsat == true){
			lletra = ("123A456B789C*0#DNF"[index]);
			temps_lletra = ara_lletra;
			
			if (lletra == 'D' ) {
				menu(&status); //Menu Principal	
			}//if menu
		}// Id boto polsat
	}//IF control polsacions
}//funció menu	

//uint32_t lastKeyPressed = 0;

void loop() {
	lcd.setBacklight(255);//activem llum
	int aigua =	status.aigua;//
	int gas =	status.gas;//
	int moviment =	status.moviment;//
	int alarma =	status.alarma;//
	
	
	cridar_menu(&status);
	obrir_tancar_dins(&status);
	Detectar_tot(&status);



	delay(1000);
}	