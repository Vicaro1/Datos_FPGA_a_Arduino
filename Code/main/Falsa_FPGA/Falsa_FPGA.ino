/*Funcion:        Codigo que simula el envio de datos por parte de la FPGA.
 
  Funcionamiento: Cada pocos segundos envia una cadena de caracteres correspondientes a dos numeros Hexadecimales y un simbolo de control por el puerto serie.
  
  Formato:  XXYYZZTT3D
            XXYY: Corresponde al primer numero (Entrada de abejas).
            ZZTT: Corresponde al segundo numero (Salida de abejas).
            3D:   Corresponde al valor en hexadecimal del caracter ASCII =. 
            
  Falso Delay:    Creamos una "tarea" empleando la funcion milis y dos variables.
*/


// Variables para el falso delay y la velocidad de comunicacion.
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;
unsigned int  velocidad = 9600;

// En el setup se incia la comunicacion serial a la velocidad establecida y simulando el formato de la FPGA real.
void setup() {
Serial.begin(velocidad, SERIAL_8E1);
}

// En el loop se mandaran dos datos separados por un segundo.
void loop() {
  tiempo2 = millis();            // Empezamos a contar los milisegundos desde que el arduino empieza a funcionar.
  if(tiempo2 > (tiempo1+3000)){  // Si los milisegundos actuales superan al valor asignado se ejecuta el if.
    tiempo1 = millis();          // Actualizamos el tiempo actual.
    Serial.print("=0201");       // Primer envio de datos.
    delay(1000);                 // Esperamos un segundo.
    Serial.print("=A01C");       // Segundo envio de datos.
  }

}
