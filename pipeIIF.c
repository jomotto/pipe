
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define CLOSE_ALL(_fd_) close(_fd_[0]); close(_fd_[1])
#define MAXLINE 127

#define VERSION "2018-07-20_J"

int g_hijo;

int printOut( char *p_buffer)
{
	char szBuffer[MAXLINE];
	
	memset( szBuffer, '\0', sizeof(szBuffer) );
	
	strcpy( szBuffer, p_buffer);
	write( 1, szBuffer, strlen(szBuffer)+1 );

}

int nieto( int fdHijo[2] )
{
	FILE	*fin;
	char	lineNR[MAXLINE];
	char	lineNW[MAXLINE];
	char	bufferN[MAXLINE];
	char	*ptrNChar;
	int	sendConfig;

	printOut("\n-N Hellow Nietooooooooooooooo\n");
	printf("\n-N Nieto [g_hijo = %d]\n", g_hijo );

	memset ( &lineNR, '\0', sizeof(lineNR) );
	memset ( &lineNW, '\0', sizeof(lineNW) );

	fin = fopen ( "pipeIIB.cfg", "r" );
	if ( !fin )
	{
		printOut("\n-N Error fopen pipeIIB.cfg");
		fclose ( fin );
	}

	sleep(4);

	read ( fdHijo[0], lineNR, MAXLINE);
	printf("-N NietoA [%s]\n", lineNR);
	sendConfig = 0;

	if ( fin )
	{
		while (fgets (bufferN, sizeof(bufferN), fin ) )
		{
			*(bufferN+strlen(bufferN)-1) = '\0';
			
			if ( *bufferN == '#' )
			{
				continue;
			}
			else if ( strstr (bufferN, "SENDCONFIG=Y") )
			{
				sendConfig = 1;
			}
			else if ( strstr (bufferN, "NEWCONFIG=") )
			{
				strcpy(lineNW, "N2H0");
				if (sendConfig == 1)
				{
					ptrNChar = strchr(bufferN, '=')+1;
					strcpy ( lineNW, ptrNChar );
				}
			}
		}
	}
	else
	{
		strcpy(lineNW, "N2H1");
	}

	if ( sendConfig == 1 )
	{
		if ( write(fdHijo[1], lineNW, strlen(lineNW)) != strlen(lineNW) )
		{
			printOut("\n-N Error escribiendo del Nieto al hijo\n");
		}
		else
		{
			printf("-N N2H |%d|[%s] \n", strlen(lineNW), lineNW);
		}
	}

	printOut("\n-N Bye Bye Nietooooooooooooooo\n");
	fclose ( fin );
}

void hijo ( int fdFather[2] )
{
	int	fdHijo[2];
	char	lineHR[MAXLINE];
	char	lineHW[MAXLINE];
	pid_t	id_nieto;	/* PID del proceso nieto */
	int	estadoHijoNieto;	/* Estado de salida */
	int	numHRead;
	char	outH[MAXLINE];

	g_hijo = 2;


	while ( 1 )
	{
		memset ( &lineHR, '\0', sizeof(lineHR) );
		memset ( &lineHW, '\0', sizeof(lineHW) );

		sleep (4);

		numHRead = read ( fdFather[0], lineHR, MAXLINE);
		
		if ( numHRead > 0)
		{
			printf("-H HijoA DESDE PADRE [%s] BytesRead [%d]\n", lineHR, numHRead);
			g_hijo = atoi( lineHR );
		}
		else if ( numHRead == -1 )
		{
			// case -1 means pipe is empty and errono
			// set EAGAIN
			if (errno == EAGAIN) {
				printOut("\n-H Hijo:(pipe empty)\n");
				sleep(1);
			}
			else
			{
				//perror("read");
				printOut("\n-H Hijo: Error reading pipe\n");
				exit(4);
			}
			g_hijo = -1;
		}
		else if ( numHRead == 0 )
		{
			printOut("\n-H Hijo: End of conversation\n");

			// read link
			close(fdFather[0]);
			exit(0);
			g_hijo = -2;
		}

		/* creacion de un segundo pipe */
		if ( pipe ( fdHijo ) == -1 )
		{
			perror ( "pipe" );
			exit ( -1 );
		}




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

				CLOSE_ALL ( fdHijo );

				exit (-1);
				break;
			default:
				printOut("\n-N Hellow Hijooooooooooooooo\n");

				strcpy(lineHW, "H2N");
				if ( write(fdHijo[1], lineHW, strlen(lineHW)) != strlen(lineHW) )
				{
					printOut("\n-H Error escribiendo del hijo al Nieto\n");
				}

				/////////////////////////////////////////////////////////////////////////////
				// Espera la salida del Nieto para leer
				/////////////////////////////////////////////////////////////////////////////
				while (waitpid ( id_nieto, &estadoHijoNieto, 0) !=  id_nieto);
				if (WIFSIGNALED (estadoHijoNieto))
					printf ("-H El proceso hijo ha recibido la senial d\n", WTERMSIG (estadoHijoNieto));
				if (WIFEXITED (estadoHijoNieto))
				{
					printf ("-H Estado de salida del proceso nieto: %d\n", WEXITSTATUS (estadoHijoNieto));
					if (WEXITSTATUS (estadoHijoNieto) == 1)
						//printf ("-H PID Nieto > PID Hijo.\n");
						printOut("\n-H PID Nieto > PID Hijo.\n");
					else
						printOut("\n-H PID Hijo > PID Nieto.\n");
				}
				/////////////////////////////////////////////////////////////////////////////

				memset ( &lineHR, '\0', sizeof(lineHR) );

				fcntl( fdHijo[0], F_SETFL, fcntl(fdHijo[0], F_GETFL) | O_NONBLOCK );

				numHRead = read ( fdHijo[0], lineHR, MAXLINE);
				
				
				
				
				
				
				/////////////////////////////////////////////////////////////////////////////
				// control de lectura del pipe HIJO
				/////////////////////////////////////////////////////////////////////////////
				if(numHRead > 0)
				{
					printf("-H Hijo DESDE NIETO [%s] BytesRead [%d]\n", lineHR, numHRead );
					strcpy(lineHW, lineHR);
					if ( write(fdFather[1], lineHW, strlen(lineHW)) != strlen(lineHW) )
					{
						printOut("\n-H Error escribiendo del hijo al Padre\n");
					}
				} 
				else if ( numHRead == -1 )
				{
					// case -1 means pipe is empty and errono
					// set EAGAIN
					if (errno == EAGAIN) {
						printOut("\n-H Hijo:(pipe empty)\n");
						sleep(1);
					}
				}
				/////////////////////////////////////////////////////////////////////////////

				
				
				
				
				
				
				
	
				/* Hijo */
//				CLOSE_ALL ( fdFather );
				CLOSE_ALL ( fdHijo );
				printOut("\n-H Bye Bye Hijooooooooooooooo\n");

		} // switch
	} // while 1
	
	CLOSE_ALL ( fdFather );

}

int main( int argc, char *argv[] )
{
	int fdFather[2];
	char	lineFR[MAXLINE];
	char	lineFW[MAXLINE];
	pid_t	id_padre;	/* PID del proceso padre */
	pid_t	id_hijo;	/* PID del proceso hijo */
	int	estado;	/* Estado de salida */
	char	buf[MAXLINE];
	char	out[MAXLINE];
	int	numRead;


	printf ("%s [%s]\n", argv[0], VERSION);
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

			while ( 1 )
			{
				memset ( &lineFR, '\0', sizeof(lineFR) );
				memset ( &lineFW, '\0', sizeof(lineFW) );

///////////////////////////////////////////////////////////////////
				strcpy(buf, "Ingrese numero:");
				write( 1, buf, strlen(buf) );


				fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
				sleep(2);

				memset( buf, '\0', sizeof(buf) );
				numRead = read(0,buf,4);
				if(numRead > 0)
				{
					memset( out, '\0', sizeof(out) );
					*(buf+strlen(buf)-1) = '\0';
					sprintf( out, "-P [%s] 2 pipe\n", buf);
					write( 1, out, strlen(out)+1 );
					strcpy(lineFW, buf);
				}
				else
				{
					//memset( out, '\0', sizeof(out) );
					//strcpy( out, "-P No ingreso Datos A\n");
					//write( 1, out, strlen(out)+1 );
					printOut("\n-P No ingreso Datos A\n");
				}
///////////////////////////////////////////////////////////////////


				if ( strlen(lineFW) )
				{
					if ( write(fdFather[1], lineFW, strlen(lineFW)) != strlen(lineFW) )
					{
						//memset( out, '\0', sizeof(out) );
						//strcpy( out, "-P Error escribiendo del padre al hijo\n" );
						//write( 1, out, strlen(out)+1 );
						printOut("\n-P Error escribiendo del padre al hijo\n");
					}
					else
					{
						memset( out, '\0', sizeof(out) );
						sprintf( out, "-P P2H [%s]\n", lineFW);
						write( 1, out, strlen(out)+1 );
					}
				}

				/////////////////////////////////////////////////////////////////////////////
				// Espera la salida del hijo para leer
				/////////////////////////////////////////////////////////////////////////////
				/*
				while (waitpid (id_hijo, &estado, 0) != id_hijo);

				if (WIFSIGNALED (estado))
					printf ("El proceso hijo ha recibido la senial %d\n", WTERMSIG (estado));

				if (WIFEXITED (estado))
				{
					printf ("Estado de salida del proceso hijo: %d\n", WEXITSTATUS (estado));

					if (WEXITSTATUS (estado) == 1)
						printf ("PID hijo > PID padre.\n");
					else
						printf ("PID padre > PID hijo.\n");
				}
				*/
				/////////////////////////////////////////////////////////////////////////////

				memset ( &lineFR, '\0', sizeof(lineFR) );
				
				fcntl( fdFather[0], F_SETFL, fcntl(fdFather[0], F_GETFL) | O_NONBLOCK );

				numRead = read ( fdFather[0], lineFR, MAXLINE);


				/////////////////////////////////////////////////////////////////////////////
				// control de lectura del pipe
				/////////////////////////////////////////////////////////////////////////////
				if(numRead > 0)
				{
					memset( out, '\0', sizeof(out) );
					//strcpy( out, "No ingreso Datos A\n");
					sprintf( out, "-P H2P [%s]\n", lineFR);
					write( 1, out, strlen(out)+1 );
				} 
				else if ( numRead == -1 )
				{
					// case -1 means pipe is empty and errono
					// set EAGAIN
					if (errno == EAGAIN) {
						//printf("Parent:(pipe empty)\n");
						//memset( out, '\0', sizeof(out) );
						//strcpy( out, "-P Parent:(pipe empty)\n");
						//write( 1, out, strlen(out)+1 );
						printOut("\n-P Parent:(pipe empty)\n");
						sleep(1);
					}
					else
					{
						//perror("read");
						//memset( out, '\0', sizeof(out) );
						//strcpy( out, "-P Error reading pipe\n");
						//write( 1, out, strlen(out)+1 );
						printOut("\n-P Error reading pipe\n");
						exit(4);
					}
				}
				else if ( numRead == 0 )
				{
					//memset( out, '\0', sizeof(out) );
					//strcpy( out, "-P End of conversation\n");
					//write( 1, out, strlen(out)+1 );
					printOut("\n-P End of conversation\n");
					// read link
					close(fdFather[0]);
					exit(0);
				}
				/////////////////////////////////////////////////////////////////////////////



			}

			CLOSE_ALL ( fdFather );

			printf("-P Padre END\n");
			exit (-1);
			break;
	}

	return 0;
}
