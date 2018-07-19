#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define CLOSE_ALL(_fd_) close(_fd_[0]); close(_fd_[1])


void hijo ( int fd1[2] )
{
	int fd2[2];

	/* creacion de un segundo pipe */
	if ( pipe ( fd2 ) == -1 )
	{
		perror ( "pipe" );
		exit ( -1 );
	}

	/* creacion de un proceso P2, nieto del proceso Principal */
	switch ( fork () )
	{
		case -1:
			perror ( "fork" );
			exit (-1);
			break;
		case 0:
			/* cierre de los descriptores del pipe 1 */
			CLOSE_ALL ( fd1 );
			/* Redireccion del descriptor de entrada estandar a partir de la tuberia 2 */
			dup2 ( fd2[0], STDIN_FILENO ); /* stdin */
			/* cierre de los descriptores de la tuberia 2 */
			CLOSE_ALL (fd2);
			/* ejecucion de: tail */
			execlp ( "tail", "tail", NULL );
			perror ( "execlp" );
			exit (-1);
			break;
		default:
			/* Redireccion del descriptor de entrada
			** estandar a partir de la tuberia 1 */
			dup2 ( fd1[0], STDIN_FILENO ); /* stdin */
			/* Redireccion del descriptor de salida
			** estandar hacia la tuberia 2 */
			dup2 ( fd2[1], STDOUT_FILENO ); /* stdout */
			/* cierre de los descriptores
			** de la tuberia 1 y 2 */
			CLOSE_ALL ( fd1 );
			CLOSE_ALL ( fd2 );
			/* ejecucion de: grep -v root */
			execlp ( "grep", "grep", "-v", "root", NULL );
			perror ("execlp");
			exit (-1);
			break;
	}
}

void main()
{
	int fd[2];

	/* creacion de la primer tuberia: tube 1 */
	if ( pipe (fd) == -1)
	{
		perror ( "pipe" );
		exit (-1);
	}

	/* creacion de un primer hijo P1 que ejecutara la funcion hijo (fd) */
	switch ( fork () )
	{
		case -1:
			perror ( "fork" );
			exit (-1);
			break;
		case 0:
			hijo ( fd );
			break;
		default:
			/* Redireccion del descriptor de salida estandart hacia el pipe 1 */
			dup2 ( fd[1], STDOUT_FILENO); /* stdout */
			/* cierre de los descriptores del pipe 1 */
			CLOSE_ALL (fd);

			/* ejecucion de . ps aux */
			execlp ( "ps", "ps", "aux", NULL );
			perror ( "execlp" );
			exit (-1);
			break;
	}
}
