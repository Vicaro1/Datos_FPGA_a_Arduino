# Datos_FPGA_a_Arduino
Este proyecto busca mostrar los datos en procedentes de una FPGA por comunicación UART mediante Arduino.
- Función:        Código que muestra por una pantalla LCD los valores que llegan por el puerto serie y posteriormente
                  son almacenados en una tarjeta SD.
  
- Funcionamiento: Constantemente el arduino esta recibiendo datos por el puerto serie con un formato 
                  concreto, esos datos son tratados y posteriormente mostrados en la pantalla LCD.
                  
- Formato:        XYZT=

      =:  Corresponde al caracter de fin de cadena.

  XY: Corresponde al primer número (Entrada de abejas).

      X: Primer digito hexadecial.
      Y: Segundo digito hexadecial.

  ZT: Corresponde al segundo número (Salida de abejas).

      Z: Primer digito hexadecial.
      T: Segundo digito hexadecial.
                
- Conversión:

  Para pasar de Hexadecimal a decimal se el siguiente algoritmo:

      Si el dígito que se esta leyendo es una letra (A,B,C,D,E,F) se escribe el equivalente decimal (10,11,12,13,14,15)
      Si el dígito que se esta leyendo es un número se deja como está.
      Posteriormente se hace la conversion adecuada para obtener el número deseado.
          Input   = X*16^1 + Y*16*0 = X*16 + Y*1 = X*16 + Y
          Output  = Z*16^1 + T*16*0 = Z*16 + T*1 = Z*16 + T
  
- Falso Delay:    Creamos una "tarea" empleando la funcion milis y dos variables.

- RealTimeClock:  Emplearemos un modulo RTC para saber la fecha y hora en todo momento.

- Modulo SD:      Usando un modulo SD en conjunto con el modulo RTC crearemos un datalogger que almacene los datos al final del día.
