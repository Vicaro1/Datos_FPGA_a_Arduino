/** Encabezado:
 * @file main.ino
 * @brief Código de Datos_FPGA_a_Arduino
 * @author [Victor Caro Pastor](https://github.com/Vicaro1)
 * @version  V3.2
 * @date  05-04-2023
*/

// Se incluirá una libreria para la comunicación I2C, adaptador LCD I2C, reloj RTC y modulo SD.
#include <Wire.h>               
#include <LiquidCrystal_I2C.h>  
#include "RTClib.h"             
#include <SD.h>                 

// Variables para los falsos delay y la velocidad de comunicación.
// 9600 bits por segundo/8 = 1200 bytes por segundo

const uint16_t BAUDIOS = 9600; 
uint64_t g_milisegundos_transcurridos_1 = 0;
uint64_t g_milisegundos_transcurridos_2 = 0;
uint64_t g_milisegundos_transcurridos_3 = 0;
uint64_t g_milisegundos_transcurridos_4 = 0;


// Variables que guardarán el número de abejas que entran y salen en cada instante.
int16_t g_abejas_entrantes=0;
int16_t g_abejas_salientes=0;
int16_t g_abejas_panal=0;

// Variable que guardará la cadena de texto con la información, varible del contador de bytes de la cadena 
// y la bandera de cadena completa.
String g_cadena_datos="";
bool g_b_cadena_datos_completa=false;
uint8_t g_contador_bytes=0;
int cuenta = 0;

// El adaptador I2C lleva un controlador PCF8574T cuya direccion I2C es 0x27 y estamos empleando 
// una pantalla de 20 columnas y 4 filas.
LiquidCrystal_I2C lcd(0x27,20,4);  

// Al modulo de reloj se le asignara un nombre para identificarlo y un nombre para almacenar la fecha y hora.
RTC_DS1307 rtc;
DateTime fecha_y_hora;

// Para interactuar con un archivo dentro de la tarjeta declaramos una variable del tipo File.
File mi_archivo;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCIONES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Esta función se encarga de colocar un 0 delante del número si este tiene solo un digito. 
 *  @param numero: Dígito se se comprueba.
 *  @type numero: int.
 *  @return numeros: Cadena con el 0 añadido o no al valor de numero.
 *  @rtype numeros: String.
 */
String coloca_2_digitos(int numero) 
{
  String numeros;
  if (numero < 10) numeros = "0" + String(numero);
  else numeros = String(numero);
  return numeros;
}

/* Esta función se encarga de convertir el caracter ASCII (que representa un valor hexadeciamal) en decimal 
 * a partir de una cadena de datos completa .
 *  @param cadena: Cadena de caracteres con todos los datos.
 *  @type cadena: String.
 *  @param x: Posicion del caracter a evaluar en la cadena de datos.
 *  @type x: int8_t.
 *  @return caracter_decimal: Valor decimal del caracter hexadecimal evaluado.
 *  @rtype caracter_decimal: int16_t.
 */
int16_t conversor_hex_int(String cadena,int8_t x)
{
  String caracter = cadena.substring(x, x+1);
  int16_t caracter_decimal = 0;
  if(caracter=="A") caracter_decimal=10;
  else if(caracter=="B") caracter_decimal=11;
  else if(caracter=="C") caracter_decimal=12;
  else if(caracter=="D") caracter_decimal=13;
  else if(caracter=="E") caracter_decimal=14;
  else if(caracter=="F") caracter_decimal=15;
  else caracter_decimal=caracter.toInt();
  return caracter_decimal;
}


/* Esta función se encarga de leer los caracteres entrantes por el puerto serie e ir
 * añadiendolos a una cadena de datos hasta completar los 5 bytes del mensaje, activando una variable
 * bandera.
 */
void serialEvent()
{
  while(Serial.available())
  {              
    char caracter = (char)Serial.read();  // Se lee el byte como caracter y se guarda en una variable.    
    g_cadena_datos += caracter;           // Se concatena dicha variable a la cadena de datos.    
    ++g_contador_bytes;                   // Se incrementa el contador en 1.
    if(g_contador_bytes == 5)             // Si se reciben los 5 bytes del mensaje.
    {              
      g_contador_bytes = 0;               // Se reinicia el contador.   
      g_b_cadena_datos_completa = true;   // Se activa la variable bandera. 
      Serial.println("Cadena");   
    }
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// En el setup se incian todos los modulos, el puerto UART y se escribe el cuerpo del LCD.
// Se configura la comunicación UART con 8 bit, paridad par y 1 bit de parada.
void setup() 
{
  Serial.begin(BAUDIOS, SERIAL_8E1);  // Se configura la comunicación UART con 8 bit, paridad par y 1 bit de parada.
  rtc.begin();                        
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   // Se fija la fecha y hora de compilación.
  g_cadena_datos.reserve(5);                        // Reservamos 5 bytes de buffer para la cadena de datos.
  
  
  // Se inicia la pantalla LCD con la luz de fondo encendida y se escribe el texto informativo.
  lcd.init(); lcd.backlight();                       
  lcd.setCursor(0, 1); lcd.print("Entrada: ");  
  lcd.setCursor(0, 2); lcd.print("Salida: ");
  lcd.setCursor(0, 3); lcd.print("En el Panal: "); 
  
  // Si el SD no se conecta lo indica mediante el LED integrado del arduino.
  pinMode(LED_BUILTIN,OUTPUT);
  if (!SD.begin(4)) 
  {
    digitalWrite(LED_BUILTIN,HIGH);
    return;
  }
  // Se abre el archivo Data y se escriben las cabezeras de las columnas.
  mi_archivo = SD.open("Data.txt", FILE_WRITE);
  if (mi_archivo) 
  { 
    mi_archivo.println("Fecha,Hora,Abejas Entrantes,Abejas Salientes,Abejas en el panal");
    mi_archivo.close();  
  }       
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// En el loop se recibiran los datos por el puerto serie continuamente y se mostraran por la pantalla LCD cada 4 segundos.
void loop() 
{
  // Se obteniene la hora y fecha actuales.
  fecha_y_hora = rtc.now();   
  // Se guardan la fecha y hora formateada con dos dígitos mediante la función coloca_2_digitos
  String hora = coloca_2_digitos(fecha_y_hora.hour());
  String minuto = coloca_2_digitos(fecha_y_hora.minute());
  String segundo = coloca_2_digitos(fecha_y_hora.second());
  String dia = coloca_2_digitos(fecha_y_hora.day());
  String mes = coloca_2_digitos(fecha_y_hora.month());
  String meses[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", 
                      "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
  
  // Mostramos por el LCD la fecha actualizada constantemente.
  lcd.setCursor(0, 0);
  lcd.print(hora + ":" + minuto + ":" + segundo + "  " +dia + "/" + mes);
  
  // Si la cadena de datos esta completa procedemos a interpretarla llamando a la función
  // conversora para obtener el dígito en decimal de cada caracter hexadecimal y posteriormente calcular el valor.
  if (g_b_cadena_datos_completa) 
  {                                   
    int16_t entrantes = conversor_hex_int(g_cadena_datos,1)*16 + conversor_hex_int(g_cadena_datos,2);       
    int16_t salientes = conversor_hex_int(g_cadena_datos,3)*16 + conversor_hex_int(g_cadena_datos,4);

    g_abejas_entrantes = g_abejas_entrantes + 1;
    //g_abejas_entrantes = g_abejas_entrantes + entrantes;    // Calculamos el total de abejas que entran.
    g_abejas_salientes = g_abejas_salientes + salientes;    // Calculamos el total de abejas que salen.
    g_abejas_panal = g_abejas_entrantes - g_abejas_salientes;      // Calculamos el total de abejas que permanecen en el panal.

    // Se vacia la cadena de datos actual y la bandera de cadena completa se desactiva.
    g_cadena_datos = ""; 
    g_b_cadena_datos_completa = false;
  }
  
  
  // Una vez leidos e interpretados los datos se muestran en la pantalla LCD cada 4 segundos.
  g_milisegundos_transcurridos_2 = millis();              
  if(g_milisegundos_transcurridos_2 > (g_milisegundos_transcurridos_1+4000))
  {  
    g_milisegundos_transcurridos_1 = millis();   
    // Al principio de cada impresión se limpiara lo que se escribió previamente y se escriben los nuevos valores.
    lcd.setCursor(10, 1); lcd.print("          ");
    lcd.setCursor(10, 1); lcd.print(g_abejas_entrantes);
    lcd.setCursor(10, 2); lcd.print("          ");
    lcd.setCursor(10, 2); lcd.print(g_abejas_salientes);
    lcd.setCursor(13, 3); lcd.print("          ");
    lcd.setCursor(13, 3); lcd.print(g_abejas_panal); 
  }
  
  // Si han pasado cinco minutos se vuelcan los datos en la tarjeta SD.
  g_milisegundos_transcurridos_4 = millis();              
  if(g_milisegundos_transcurridos_4 > (g_milisegundos_transcurridos_3+30000))
  {
    g_milisegundos_transcurridos_3 = millis();
    mi_archivo = SD.open("Data.txt", FILE_WRITE);
    if (mi_archivo) 
    { 
      mi_archivo.println(dia + " de " + meses[fecha_y_hora.month()-1] +
                         "," + hora + ":" + minuto + ":" + segundo + "," + g_abejas_entrantes +
                         "," + g_abejas_salientes + "," + g_abejas_panal);
      ++cuenta;
      mi_archivo.close();  
    }
  }

  // Si han pasado 24 horas reseteamos todos los contadores
  if (cuenta==288)
  {
    // Una vez registrados los datos se reinician todas las variables contadoras
    g_abejas_entrantes = 0;    
    g_abejas_salientes = 0;      
    g_abejas_panal = 0;
    cuenta = 0;      
   }    
}

