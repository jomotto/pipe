#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])

{
    char buf[20];

    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    printf ("Ingrese un numero [4 seg]:");
    sleep(4);


    memset( buf, '\0', sizeof(buf) );
    int numRead = read(0,buf,4);
    if(numRead > 0){
        printf("You said: %s\n", buf);
    }
}
