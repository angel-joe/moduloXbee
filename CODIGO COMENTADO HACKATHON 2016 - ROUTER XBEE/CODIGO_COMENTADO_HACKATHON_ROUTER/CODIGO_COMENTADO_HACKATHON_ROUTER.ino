/*-- CODIGO MODULO ROUTER - HACKATHON POR LA AMAZONIA 2016 - COMENTADO --  */

#include "SIM900.h"                       // IMPORTAR LA LIBRERIA DEL SIM900 - GSM
#include "inetGSM.h"                      // IMPORTAR LA LIBRERIA DEL GSM
#include <XBee.h>                         // IMPORTAR LA LIBRERIA DEL MODULO XBEE

XBee x = XBee();                          // INSTANCIAMOS LA CLASE XBEE 

ZBRxResponse frame = ZBRxResponse();      // INSTANCIAMOS LA CLASE RESPUESTA EN VARIABLE FRAME

InetGSM inet;                             // Utilizando la libreria del GSM

char msg[50];                             // LIBRERIA DE ARREGLO DEL SIM900, TIENE 50 CARACTERES
int numdata;                              // DECLARAMOS UN VARIABLE DE TIPO ENTERO
char inSerial[50];                        // LIBRERIA DE ARREGLO DEL SIM900, TIENE 50 CARACTERES
int i = 0;                                // DECLARAMOS UNA VARIABLE DE TIPO ENTERO INICIADA EN CERO
boolean started = false;                  // INICIA EL ESTADO EN FALSO
char datastreamId[] = "1";                // LIBRERIA DEL ARREGLO DEL SIM900 PARA QUE INICIE EN 1


/* --------------   AJUSTE DE THINGSPEAK ------------------- */

char thingSpeakAddress[] = "api.thingspeak.com";   // URL DE LA PLATFORMA WEB DE THIGSPEAK
char writeAPIKey[] = "Z58BEUM3S5GIDWV9";           // APIKEY GENERADO AL REGISTRO DE UNA CUENTA EN LA PLATAFORMA THINGSPEAK
const int updateThingSpeakInterval = 60 * 1000;    // Intervalo de tiempo en milisegundos para actualizar ThingSpeak (número de segundos * 1000 = intervalo)
char sentMsg[50];

float ECcurrent = 0;      // 1er sensor de conductividad
float temperature = 0;    // 2do sensor de temperatura
float ph = 0;             // 3er sensor de ph
float vibra = 0;          // simulacion de sensor de vibracion mediante potenciometro y dos diodos led

void startupGSM900() {

  if (gsm.begin(115200))          // INICIA EN VELOCIDAD DE 115200
  {
    started = true;
  }

  if (started)                    // INICIA
  {
    gsm.SimpleWrite("AT");
    delay(1000);
    gsm.WhileSimpleRead();
    gsm.SimpleWriteln("AT+CIFSR");
    delay(5000);
    gsm.WhileSimpleRead();   // que lea hasta el serial estee vacio -- buffer.
  }
}

/* ------ METODO DE ENVIO DE DATOS A LA PLATAFORMA EN LA NUBE -- inicio de ajuste para el envio de dato hacia la nube */

void thingspeakPost()
{
  char itoaBuffer[8];
  char end_c[2];
  end_c[0] = 0x1a;
  end_c[1] = '\0';

  /* ---------- Temperatura - introduciendo un caracter para el envio de dato -------- */

  char TempData[50];
  String strTm;
  strTm = String(temperature, 2);
  strTm.toCharArray(TempData, 50);
  /*****************************************************************************************/


  /* ---------- PH - introduciendo un caracter para el envio de dato --------------------*/

  char dataPh[50];
  String strPh;
  strPh = String(ph, 2);
  strPh.toCharArray(dataPh, 50);
  /*****************************************************************************************/

  /* -------- CE - introduciendo un caracter para el envio de dato --------------------*/

  char dataCe[50];
  String strCe;
  strCe = String(ECcurrent, 2);
  strCe.toCharArray(dataCe, 50);


  /* -------- VIBRACION - introduciendo un caracter para el envio de dato --------------------*/

  char dataVr[50];
  String strVr;
  strVr = String(vibra, 2);
  strVr.toCharArray(dataVr, 50);

  if (inet.connectTCP(thingSpeakAddress, 80))                                 // SE CONECTA POR TCP A LA PLATAFORMA WEB POR EL PUERTO 80
  {
    gsm.SimpleWrite("POST /update HTTP/1.1\r\n");                             // ACTUALIZA LA PLATAFORMA WEB DE THINGSPEAK
    gsm.SimpleWrite("Host: api.thingspeak.com\r\n");
    gsm.SimpleWrite("Connection: close\r\n");                                 // CONEXION
    gsm.SimpleWrite("X-THINGSPEAKAPIKEY: ");                                  // PREPARA PARA QUE LEA EL APIKEY
    gsm.SimpleWrite(writeAPIKey);                                             // RE-ESCRIBE EL APIKEY
    gsm.SimpleWrite("\r\n");
    gsm.SimpleWrite("Content-Type: application/x-www-form-urlencoded\r\n");   // RECONOCIMIENTO DEL APIKEY
    gsm.SimpleWrite("Content-Length: ");                                      // CONTANDO LA LONGITUD, PARA ENVIAR LOS PARAMETROS

    sprintf(sentMsg, "field1=%s&field2=%s&field3=%s&field4=%s", dataCe , TempData, dataPh, dataVr);  // ENVIA DATOS A ESTOS CAMPOS EN LA NUBE

    itoa(strlen(sentMsg), itoaBuffer, 10);
    gsm.SimpleWrite(itoaBuffer);
    gsm.SimpleWrite("\r\n\r\n");
    gsm.SimpleWrite(sentMsg);
    gsm.SimpleWrite("\r\n\r\n");
    gsm.SimpleWrite(end_c);
  }

  else
  {
    startupGSM900();      // METODO DE INICIO DEL GSM900
  }
}


/*----- CONFIGURACION INICIAL, SOLO SE EJECUTA UNA SOLA VEZ CUANDO ARRANCA EL ARDUINO O SI LO REINICIAN -----*/

void setup()
{
  Serial.begin(9600);     // INICIO DE VELOCIDAD - SERIAL
  startupGSM900();        // INICIO DEL GSM900

  ECcurrent = 0;          // 1er sensor de conductividad
  temperature = 0;        // 2do sensor de temperatura
  ph = 0;                 // 3er sensor de ph
  vibra = 0;              // SIMULACION DE SENSOR DE VIBRACION POR UN POTENCIOMETRO Y 2 DIODOS LED

  x.setSerial(Serial);
  pinMode(13, OUTPUT);
}


/*--------- FUNCION PRINCIPAL - LUGAR DONDE SE SE EJECUTARA NUESTRO CODIGO -----*/

void loop()
{

  x.readPacket();
  if (x.getResponse().isAvailable())        // SI ESTA HABILITADO,ENTONCES
  {
    x.getResponse().getZBRxResponse(frame); // METODO DE OBTENCION DE DATOS - RECONSTRUCCION TRAMA

                  /*  RECONSTRUCCION DE LOS FRAMES - TRAMAS */
                  
    byte p0 = frame.getData(1);         // OBTIENE DATO DEL PRIMER ARREGLO
    byte p1 = frame.getData(2);         // OBTIENE DATO DEL SEGUNDO ARREGLO
    int ECu = p0 * 256 + p1;            // CONVIERTE A ENTERO; INT = 2 BYTE
    byte p2 = frame.getData(3);         // OBTIENE DATO DEL TERCER ARREGLO
    byte p3 = frame.getData(4);         // OBTIENE DATO DEL CUARTO ARREGLO
    int temp = p2 * 256 + p3;           // CONVIERTE A ENTERO; INT = 2 BYTE
    byte p4 = frame.getData(5);         // OBTIENE DATO DEL QUINTO ARREGLO
    byte p5 = frame.getData(6);         // OBTIENE DATO DEL SEXTO ARREGLO
    int phs = p4 * 256 + p5;            // CONVIERTE A ENTERO; INT = 2 BYTE
    byte p6 = frame.getData(7);         // OBTIENE DATO DEL SEPTIMO ARREGLO
    byte p7 = frame.getData(8);         // OBTIENE DATO DEL OCTAVO ARREGLO
    int vibr = p6 * 256 + p7;           // CONVIERTE A ENTERO; INT = 2 BYTE

    temperature = temp / 10.0;          // CONVERSION A VALOR FLOAT LA TEMPERATURA
    ECcurrent = ECu / 10.0;             // CONVERSION A VALOR FLOAT LA CONDUCTIVIDAD
    ph = phs / 10.0;                    // CONVERSION A VALOR FLOAT EL PH
    vibra = vibr / 10.0;                // CONVERSION A VALOR FLOAT LA VIBRACION - DEMO POR POTENCIOMETRO Y DIODOS LED
    digitalWrite(13, HIGH);             // LED IDENTIFICADOR DE SEÑAL TRANSMITIDA - ENCIENDE CUANDO RECIBE DATOS DEL MODULO XBEE
    delay(100);                         // RETARDO DE ENCENDIDO DEL LED EN MILISEGUNDOS
    digitalWrite(13, LOW);

    thingspeakPost();                   // METODO DE ENVIO DE DATOS A LA PLATAFORMA - OPEN SOURCE
  }
}                                       // FIN LOOP




/*---------------- SEGUNDA PARTE DEL MODULO XBEE COMO MODO COORDINADOR ------------------------*/


/*-- CODIGO MODULO COORDINADOR - HACKATHON POR LA AMAZONIA 2016 - COMENTADO --  */

#include <OneWire.h>         // libreria del sensor de temperatura DS18B20
#define StartConvert 0       // comiensa la convercción con 0
#define ReadTemperature 1    // lee la temperatura 
#define SensorPin A0         // la salida analogica del ph lo ponemos al pin analogico 0 de entrada del arduino
#define ECsensorPin A1       // la salida analogica del CE(sensor de conductividad)lo ponemos al pin analogico 1 de la entrada del arduino
#define DS18B20_Pin  4       // sensor de temperatura DS18B20 designamos, al pin digital 4
OneWire ds(DS18B20_Pin);     // Utilizando la libreria de la temperatura en el pin  digital 4
#define Offset 0.00          // desviación de compensación el ph
#define samplingInterval 25  // se toma 25 muestras
#define printInterval 30000  // intervalo de 30 segundos toma datos
#include <SoftwareSerial.h>  // libreria del software serial
SoftwareSerial sw(8, 9);     // rx al pin 8 y tx al pin 9 del arduino

unsigned int AnalogValueTotal;                // para el promedio del ph
float averageVoltage, temperature, ECcurrent; // declaracion de variables de tipo float
float voltPh = 0;                             // volt iniciado en 0
float ph = 0;                                 // ph iniciado en 0

unsigned long samplingTime = millis();   // convertir el tiempo de espera en milisegundos
unsigned long printTime = millis();      // convertir el tiempo de espera en milisegundos
int pHanalog;                            // declaracion de variable tipo entero para el PH
long n;                                  // numero de muestras de tipo long

#include <XBee.h>                        // importar la libreria del modulo Xbee
#define ACK                               
XBee x = XBee();                         // instanciamos la clase Xbee
byte datos[9] {1, 0, 0, 0, 0, 0, 0, 0, 0};                  //  envio de tramas en arreglo, el primero es el identificador del modulo
XBeeAddress64 dir = XBeeAddress64(0x13A200, 0x41242778);    // direccion de 64 bits y la direccion mac del otro modulo Xbee(router)  
ZBTxRequest frame = ZBTxRequest(dir, datos, sizeof(datos)); // metodo de solicitud y tamaño de datos a enviar
const int ledAlarma = 10;                                   // diodo led para simular un sensor de sonido(parametro normal)
const int ledDimmer= 11;                                    // diodo led para simular un sensor de sonido(alteracion de parametro)

/*----- CONFIGURACION INICIAL, SOLO SE EJECUTA UNA SOLA VEZ CUANDO ARRANCA EL ARDUINO O SI LO REINICIAN -----*/

void setup() {
  Serial.begin(9600);           // INICIO DE VELOCIDAD - SERIAL
  sw.begin(9600);               // INICIO DE VELOCIDAD - SERIAL  EN EL XBEE
  x.setSerial(sw);              
  TempProcess(StartConvert);    //Comienza la comvercion del sesnor de temperatura DS18B20
  ECcurrent = 0;
  temperature = 0;
  ph = 0;
  
 printTime = millis();          // tiempo en milisegundos
 pinMode(ledAlarma, OUTPUT);    // modo del led
 pinMode(ledDimmer, OUTPUT);    // modo del led
}

/*--------- FUNCION PRINCIPAL - LUGAR DONDE SE SE EJECUTARA NUESTRO CODIGO -----*/

void loop() {
  
  if (millis() - printTime > printInterval)     // Cada 3000 milisegundos , imprimen un número, convierten el estado de los LED
  {
    printTime = millis();                       
    TempProcess(StartConvert);                  // Inicio de la convercion
    while (n < 40) 
    {   
      if (millis() - samplingTime > samplingInterval) 
      {
        pHanalog += analogRead(SensorPin);
        AnalogValueTotal += analogRead(ECsensorPin);
        n++;
        samplingTime = millis();
      }
    }

/*-----------  CALIBRACION DE SENSORES ----------------------- */

    averageVoltage = (AnalogValueTotal / (float)n) * 5000.0 / 1024;
    float TempCoefficient = 1.0 + 0.0185 * (temperature - 25.0); //formula de la compensacion de la temperatura: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
    float CoefficientVolatge = (float)averageVoltage / TempCoefficient;

    if (CoefficientVolatge <= 448)ECcurrent = 6.84 * CoefficientVolatge - 64.32;          // 1ms/cm<EC<=3ms/cm
    else if (CoefficientVolatge <= 1457)ECcurrent = 6.98 * CoefficientVolatge - 127;      // 3ms/cm<EC<=10ms/cm
    else ECcurrent = 5.3 * CoefficientVolatge + 2278;                                     // 10ms/cm<EC<20ms/cm
    ECcurrent /= 1000;                                                                    // convierte us/cm to ms/cm

    temperature = TempProcess(ReadTemperature);                                           // requiere ser mayor a 700 ms

    voltPh = ((float)pHanalog) / n * 5.0 / 1024;                                          //calibrando el ph
    ph = 3.5 * voltPh + Offset; 

    pHanalog = 0;
    Serial.print(ECcurrent);
    int valor1 = (int)(ECcurrent * 10);
    int valor2 = (int)(temperature * 10);
    int valor3 = (int)(ph * 10);
    int valor4 = analogRead(2);
    datos[1] = highByte(valor1);                          // envio de tramas, identificador de modulo Xbee
    datos[2] = lowByte(valor1);                           // envio de tramas
    datos[3] = highByte(valor2);                          // envio de tramas
    datos[4] = lowByte(valor2);                           // envio de tramas
    datos[5] = highByte(valor3);                          // envio de tramas
    datos[6] = lowByte(valor3);                           // envio de tramas
    datos[7] = highByte(valor4);                          // envio de tramas
    datos[8] = lowByte(valor4);                           // envio de tramas
   
    frame = ZBTxRequest(dir, datos, sizeof(datos));      
    x.send(frame);                                        // enviar el paquete de datos

    if(valor4 > 512)                                      // valor medio del potenciometro simulado en incremento para paraetros de los diodos leds
    {
      digitalWrite(ledAlarma, HIGH);
      digitalWrite(ledDimmer,LOW);
    }
    else if(valor4 < 511)                                 // valor medio del potenciometro simulado en incremento para paraetros de los diodos leds
    {
      digitalWrite(ledAlarma,LOW);
      digitalWrite(ledDimmer, HIGH);
    }  
    n = 0;                                                // numero de muestra, se convienrte en 0 para nueva mente leer
    Serial.print("EConductividad: ");
    Serial.print(valor1);                                 // imprime valor 1
    Serial.print("ms/cm");
    Serial.print("    Temperatura:");
    Serial.print(valor2);                                 // imprime valor 2
    Serial.print("    pH value: ");
    Serial.print(valor3);                                 // imprime valor 3
    Serial.print("   Vibracion: ");
    Serial.println(valor4);                               // imprime valor 4
  }
}

float TempProcess(bool ch)                                //devuelve la temperatura de un DS18B20 en grados centígrados C°
{  
  static byte data[12];
  static byte addr[8];
  static float TemperatureSum;
  if (!ch) 
  {
    if ( !ds.search(addr)) 
    {
      Serial.println("no more sensors on chain, reset search!");
      ds.reset_search();
      return 0;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) 
    {
      Serial.println("CRC is not valid!");
      return 0;
    }
    if ( addr[0] != 0x10 && addr[0] != 0x28) 
    {
      Serial.print("Device is not recognized!");
      return 0;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);                                // iniciar la conversión , con el power del encendido en al final
  }
  else 
  {
    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);                                   // leer el bloc
    for (int i = 0; i < 9; i++)                       // necesitamis 9 bytes
    {                    
      data[i] = ds.read();
    }
    ds.reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float tempRead = ((MSB << 8) | LSB);            // usando dos veces de la complentacion
    TemperatureSum = tempRead / 16;
  }
  return TemperatureSum;
}                                                   // fin del metodo TempProcess








