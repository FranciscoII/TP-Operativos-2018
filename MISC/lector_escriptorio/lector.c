/*
 * lector.c
 * este programa va a leer los archivos de escriptorio
 */

#include <stdio.h>
#include <string.h>
#include "string.h"
#include <stdlib.h>

enum errores
{ SINTAX_ERROR, E10001, E10002, E20001, E20002,
  E20003, E30001, E30002, E30003, E30004, E40001, E40002,
  E50001, E50002, E60001
};

//14 errores

char *palabrasReservadas[9] = { "abrir", "concentrar", "asignar", "wait",
  "signal", "close", "flush", "crear", "borrar"
};




char *tipoPalabra (char *);
void errorSintactico ();
int ejecutarLinea (char *);
int huboError (int);




int
main (int argc, char *argv[])
{

  FILE *escriptorio = fopen ("escriptorioPrueba.txt", "r");

  char line[256];
  int rtaEjecucion;
  while (fgets (line, sizeof (line), escriptorio) && line[0] != '\0')
    {
      rtaEjecucion = ejecutarLinea (line);
      if (huboError (rtaEjecucion))
	{
	  return -1;
	}
    }
/*
 *--mientras pueda leer lineas, las va a ejecutar
 *-		siempre y cuando no haya errores
 * */

  fclose (escriptorio);

  return 0;
}



//---------------------------------------------------------------------
int
huboError (int ejecucion)
{

  switch (ejecucion)
    {
    case SINTAX_ERROR:
      errorSintactico ();
      return 1;
    case E10001:
      printf ("ERROR 10001");
      return 1;
    case E10002:
      printf ("ERROR 10002");
      return 1;
    case E20001:
      printf ("ERROR 20001");
      return 1;
    case E20002:
      printf ("ERROR 20002");
      return 1;
    case E20003:
      printf ("ERROR 20003");
      return 1;
    case E30001:
      printf ("ERROR 30001");
      return 1;
    case E30002:
      printf ("ERROR 30002");
      return 1;
    case E30003:
      printf ("ERROR 30003");
      return 1;
    case E30004:
      printf ("ERROR 30004");
      return 1;
    case E40001:
      printf ("ERROR 40001");
      return 1;
    case E40002:
      printf ("ERROR 40002");
      return 1;
    case E50001:
      printf ("ERROR 50001");
      return 1;
    case E50002:
      printf ("ERROR 50002");
      return 1;
    case E60001:
      printf ("ERROR 60001");
      return 1;
    default:
      return 0;
    }
//---------------------------------------------------------------------

  void errorSintactico ()
  {
    printf ("error sintactico");
  }

//---------------------------------------------------------------------

  char *tipoPalabra (char *linea)
  {

    int i = 0;
    char *comparador = malloc (strlen (linea) + 1);

/*convierto el comparador en la supuesta palabra reservada
 de la linea*/

    while (linea[i] != ' ')
      {
	comparador[i] = linea[i];
	i++;
      }

/*verifico que tipo de palabra reservada es el comparador
o si no lo es*/

    for (int k = 0; k < 9; k++)
      {				//k<9 por la cantidad de palabras reservadas

	if (string_equals_ignore_case (comparador, palabrasReservadas[k]))
	  {
	    return comparador;
	  }
/* comparo si es igual a alguna de las palabras reservadas
 * y devuelvo la palabra en caso de ser así
 */
      }
    free (comparador);
// en caso de no ser asi devuelvo error

    return "error sintactico";
  }

//---------------------------------------------------------------------

  int ejecutarLinea (char *linea)
  {

// IMPORTANTE 100 significa ok en cada caso

// [...] aca se ejecutará dependiendo de los distintos scripts

    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[0]))
      {
	//caso abrir
	//*ejecutar abrir*
	int resultado = abrir (linea);
	// 1,2(errores) 0 100(para exito)
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[1]))
      {
	//caso concentrar
	//*ejecutar concentrar*
	int resultado = concentrar ();
	//siempre 100
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[2]))
      {
	//caso asignar
	//*ejecutar asignar*
	int resultado = asignar (linea);
	//3,4,5(errores) o 100(para exito)
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[3]))
      {
	//caso wait
	//*ejecutar wait*
	int resultado = wait ();
	//siempre 100
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[4]))
      {
	//caso signal
	//*ejecutar signal*
	int resultado = signal ();
	//siempre 100
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[5]))
      {
	//caso close
	//*ejecutar close*
	int resultado = close (linea);
	//10,11(errores) 0 100 (para exito)
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[6]))
      {
	//caso flush
	//*ejecutar flush*
	int resultado = flush (linea);
	//6,7,8,9(errores) 0 100(para exito)
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[7]))
      {
	//caso crear
	//*ejecutar crear*
	int resultado = crear (linea);
	//12,13(errores) 0 100 (para exito)
	return resultado;
      }
    if (string_equals_ignore_case
	(tipoPalabra (linea), palabrasReservadas[8]))
      {
	//caso borrar
	//*ejecutar borrar*
	int resultado = borrar (linea);
	//  14(errores) o 100 (para exito)
	return resultado;
      }
    return 0;			//error sintactico
  }

//---------------------------------------------------------------------
