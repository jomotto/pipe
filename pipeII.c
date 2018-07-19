#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define CLOSE_ALL(_fd_) close(_fd_[0]); close(_fd_[1])
#define MAXLINE 64

int nieto( int fdHijo[2] )
{
	char lineNR[MAXLINE];
	char lineNW[MAXLINE];

	printf("- Nieto\n");


	read ( fdHijo[0], lineNR, MAXLINE);
	printf("- NietoA [%s]\n", lineNR);
	

	strcpy(lineNW, "N2H");
	if ( write(fdHijo[1], lineNW, strlen(lineNW)) != strlen(lineNW) )
	{
		printf("Error escribiendo del Nieto al hijo");
	}

}

void hijo ( int fdFather[2] )
{
	int fdHijo[2];
	char lineHR[MAXLINE];
	char lineHW[MAXLINE];
	pid_t id_nieto;	/* PID del proceso nieto */
	int   estadoHijoNieto;	/* Estado de salida */

	/* creacion de un segundo pipe */
	if ( pipe ( fdHijo ) == -1 )
	{
		perror ( "pipe" );
		exit ( -1 );
	}

	read ( fdFather[0], lineHR, MAXLINE);
	printf("- HijoA [%s]\n", lineHR);
	
	/* creacion de un proceso P2, nieto del proceso Principal */
	id_nieto = fork ();

	switch ( id_nieto )
	{
		case -1:
			perror ( "fork" );
			exit (-1);
			break;
		case 0:

			/* nieto */
			/* cierre de los descriptores del pipe 1 */
			CLOSE_ALL ( fdFather );


			nieto( fdHijo );


			/* Redireccion del descriptor de entrada estandar a partir de la tuberia 2 */
			//dup2 (  fdHijo[0], STDIN_FILENO ); /* stdin */
			/* cierre de los descriptores de la tuberia 2 */
			CLOSE_ALL ( fdHijo );
			/* ejecucion de: tail */
//			execlp ( "tail", "tail", NULL );
//			perror ( "execlp" );
			exit (-1);
			break;
		default:
//			read ( fdFather[0], lineHR, MAXLINE);
			printf("- HijoB [%s]\n", lineHR);
	

			strcpy(lineHW, "H2N");
			if ( write(fdHijo[1], lineHW, strlen(lineHW)) != strlen(lineHW) )
			{
				printf("Error escribiendo del hijo al Nieto");
			}




			/////////////////////////////////////////////////////////////////////////////
			// Espera la salida del Nieto para leer
			/////////////////////////////////////////////////////////////////////////////
       			while (waitpid ( id_nieto, &estadoHijoNieto, 0) !=  id_nieto);
			if (WIFSIGNALED (estadoHijoNieto))
            			printf ("El proceso hijo ha recibido la señ%d\n", WTERMSIG (estadoHijoNieto));
        		if (WIFEXITED (estadoHijoNieto))
           		{
		           printf ("Estado de salida del proceso hijo: %d\n", WEXITSTATUS (estadoHijoNieto));
				if (WEXITSTATUS (estadoHijoNieto) == 1)
					printf ("PID Nieto > PID Hijo.\n");
				else
					printf ("PID Hijo > PID Nieto.\n");
			}
			/////////////////////////////////////////////////////////////////////////////

			read ( fdHijo[0], lineHR, MAXLINE);
			printf("- Hijo  [%s]\n", lineHR);


			strcpy(lineHW, lineHR);
			if ( write(fdFather[1], lineHW, strlen(lineHW)) != strlen(lineHW) )
			{
				printf("Error escribiendo del hijo al Padre");
			}

	
			/* Hijo */
			/* Redireccion del descriptor de entrada
			** estandar a partir de la tuberia 1 */
			//dup2 ( fdFather[0], STDIN_FILENO ); /* stdin */
			/* Redireccion del descriptor de salida
			** estandar hacia la tuberia 2 */
			//dup2 ( fdHijo[1], STDOUT_FILENO ); /* stdout */
			/* cierre de los descriptores
			** de la tuberia 1 y 2 */
			CLOSE_ALL ( fdFather );
			CLOSE_ALL ( fdHijo );
			/* ejecucion de: grep -v root */
//			execlp ( "grep", "grep", "-v", "root", NULL );
//			perror ("execlp");
			exit (-1);
			break;
	}
}

void main()
{
	int fdFather[2];
	char lineFR[MAXLINE];
	char lineFW[MAXLINE];
	pid_t id_padre;	/* PID del proceso padre */
	pid_t id_hijo;	/* PID del proceso hijo */
	int   estado;	/* Estado de salida */

	/* creacion de la primer tuberia: tube 1 */
	if ( pipe (  fdFather ) == -1)
	{
		perror ( "pipe" );
		exit (-1);
	}

	/* creacion de un primer hijo P1 que ejecutara la funcion hijo (fd) */
	id_hijo = fork ();
	switch ( id_hijo )
	{
		case -1:
			perror ( "fork" );
			exit (-1);
			break;
		case 0:
			/* sun */
			hijo ( fdFather );
			break;
		default:	
			/* father */


			strcpy(lineFW, "lineFW");
			if ( write(fdFather[1], lineFW, strlen(lineFW)) != strlen(lineFW) )
			{
				printf("Error escribiendo del padre al hijo");
			}

			/////////////////////////////////////////////////////////////////////////////
			// Espera la salida del hijo para leer
			/////////////////////////////////////////////////////////////////////////////
       			while (waitpid (id_hijo, &estado, 0) != id_hijo);
			if (WIFSIGNALED (estado))
            			printf ("El proceso hijo ha recibido la señ%d\n", WTERMSIG (estado));
        		if (WIFEXITED (estado))
           		{
		           printf ("Estado de salida del proceso hijo: %d\n", WEXITSTATUS (estado));
				if (WEXITSTATUS (estado) == 1)
					printf ("PID hijo > PID padre.\n");
				else
					printf ("PID padre > PID hijo.\n");
			}
			/////////////////////////////////////////////////////////////////////////////

			read ( fdFather[0], lineFR, MAXLINE);
			printf("- Padre [%s]\n", lineFR);

			/* Redireccion del descriptor de salida estandart hacia el pipe 1 */
//			dup2 (  fdFather[1], STDOUT_FILENO); /* stdout */
			/* cierre de los descriptores del pipe 1 */
			CLOSE_ALL ( fdFather );

			/* ejecucion de . ps aux */
//			execlp ( "ps", "ps", "aux", NULL );
			//perror ( "execlp" );
			printf("- Padre END\n");
			exit (-1);
			break;
	}
}
