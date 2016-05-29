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


