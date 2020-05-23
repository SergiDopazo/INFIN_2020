#include <stdlib.h>
#include <sys/types.h>                                                    
#include <sys/stat.h>                                                     
#include <fcntl.h>                                                        
#include <termios.h>                                                      
#include <stdio.h>                                                        
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>     
 
#define BAUDRATE B9600                                                
//#define MODEMDEVICE "/dev/ttyS0"        //Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"        //Conexió directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 /* POSIX compliant source */                       

void initialize();
void itoa();
void reverse(); 
                                                         
struct termios oldtio,newtio;     

int	ConfigurarSerie(void)	//Obrir comunicacio serie
{
	int fd;                                                           


	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );                             
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }                            

	tcgetattr(fd,&oldtio); /* save current port settings */                 

	bzero(&newtio, sizeof(newtio));                                         
	//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;                                                     

	/* set input mode (non-canonical, no echo,...) */                       
	newtio.c_lflag = 0;                                                     

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */         
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */ 

	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);
	
	sleep(2); //Per donar temps a que l'Arduino es recuperi del RESET
	
	return fd;
}               

void TancarSerie(fd) //Tancar comunicacio serie
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
                                                                                 
int main(int argc, char **argv)                                                               
{                                                                          
	int fd, i = 0, res;
	char buf[255];
	char missatge[255];
	int temps_mostreig = 0;			//Temps mostreig arduino (garantir nova mostra)
	int temps_mostreig_real = 0;	//Temps mostreig real
	float array_temp [100];			//Array circular
	int main_counter = 1; 			//Contador de lectures total
	int counter = 0;				//Posicio dins l'array circular
	int samples = 5; 				//Numero de mostres per fer la mitjana (exemple temporal)
	float maxValue = 0;				//Valor maxim
	float minValue = 70;			//Valor minim

	fd = ConfigurarSerie();
	
	initialize(&temps_mostreig, &temps_mostreig_real, &missatge);	//Funcio inicial de posada en marxa

	res = write(fd,missatge,strlen(missatge));	//Enviem missatge

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

	printf("Enviats %d bytes: ",res);		//Ensenyem per pantalla el missatge
	for (i = 0; i < res; i++)
	{
		printf("%c",missatge[i]);
	}
	printf("\n");

	//Rebem
	res = res + read(fd,buf,1);
	res = res + read(fd,buf+1,1);
	res = res + read(fd,buf+2,1);
	res = res + read(fd,buf+3,1);
	res = res + read(fd,buf+4,1);
	res = res + read(fd,buf+5,1);

	printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
	for (i = 0; i < res; i++)
	{
		printf("%c",buf[i]);			//Ensenyem la resposta per pantalla
	}
	printf("\n");
	
	while (1)		//Comencem el loop infinit on demanem mostres segons temps de mostreig donat
	{
		printf("________________________________________________________________________________\n");	//Separador
		char missatge2 [10] = "ACZ\n";	//Comanda ultima mostra
		strcpy(missatge,missatge2);

		res = write(fd,missatge,strlen(missatge));	//Enviar missatge

		if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

		printf("Enviats %d bytes: ",res);
		for (i = 0; i < res; i++)
		{
			printf("%c",missatge[i]);	//Ensenyem per pantalla el missatge
		}
		printf("\n");

		//Rebem
		res = read(fd,buf,1);
		res = res + read(fd,buf+1,1);
		res = res + read(fd,buf+2,1);
		res = res + read(fd,buf+3,1);
		res = res + read(fd,buf+4,1);
		res = res + read(fd,buf+5,1);
		res = res + read(fd,buf+6,1);
		res = res + read(fd,buf+7,1);
		res = res + read(fd,buf+8,1);
		res = res + read(fd,buf+9,1);
		
		printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
		for (i = 0; i < res; i++)
		{	
			printf("%c",buf[i]);			//Ensenyem per pantalla la resposta
		}
		printf("\n");
		        
        float last_value = 0;
        last_value = (buf[3]-48)*1000+(buf[4]-48)*100+(buf[5]-48)*10+(buf[6]-48)*1;	//Guardem el valor de la ulitma mostra (0-1023)
		
		//Procés del parpadeix
        char missatge3 [10] = "AS131Z\n";	//Comanda per encendre el led (sortida digital 13 HIGH)
		strcpy(missatge,missatge3);
		
		res = write(fd,missatge,strlen(missatge));		//Enviar missatge

		if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

		printf("Enviats %d bytes: ",res);	
		for (i = 0; i < res; i++)
		{
			printf("%c",missatge[i]);		//Ensenyem missatge per pantalla
		}
		printf("\n");

		//Rebem
		res = read(fd,buf,1);
		res = res + read(fd,buf+1,1);
		res = res + read(fd,buf+2,1);
		res = res + read(fd,buf+3,1);
		res = res + read(fd,buf+4,1);
		res = res + read(fd,buf+5,1);
		
		printf("Rebuts %d bytes: ",res);	//Mirem la resposta al buffer
		for (i = 0; i < res; i++)
		{
			printf("%c",buf[i]);			//Ensenyem resposta per pantalla
		}
		printf("\n");
		
		sleep(1);	//adormim el programa per observar el led
		
		char missatge4 [10] = "AS130Z\n";	//Comanda per apagar el led (sortida digital 13 LOW)
		strcpy(missatge,missatge4);
		
		res = write(fd,missatge,strlen(missatge));	//Enviar missatge

		if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

		printf("Enviats %d bytes: ",res);
		for (i = 0; i < res; i++)
		{
			printf("%c",missatge[i]);		//Mostrem per pantalla el missatge
		}
		printf("\n");

		//Rebem
		res = read(fd,buf,1);
		res = res + read(fd,buf+1,1);
		res = res + read(fd,buf+2,1);
		res = res + read(fd,buf+3,1);
		res = res + read(fd,buf+4,1);
		res = res + read(fd,buf+5,1);

		printf("Rebuts %d bytes: ",res);	//Mirem la resposta al buffer
		for (i = 0; i < res; i++)
		{	
			printf("%c",buf[i]);			//Ensenyem per pantalla la resposta
		}	
        printf("\n");
        
        float last_temp = 0;
		int temp_inf = 0, temp_sup = 70;
		int rang_temp = temp_sup - temp_inf;
		last_temp = (last_value/1023)*rang_temp;							//Conversio a graus celsius
		printf("Última temperatura : %0.1f ºC\n", last_temp);				//Mostrar per pantalla la ultima temperatura registrada
		printf("Número de lectures realitzades : %d\n", main_counter);		//Mostrar per pantalla el nombre de mostres realitzades totals
		
		array_temp [counter] = last_temp;	//Guardar la lectura dins l'array circular

		if (main_counter >= samples){		//Mitjana quan hi hagi el nombre demanat de mostres
			float suma = 0;
			for (int c = 0; c < samples; suma = suma + array_temp [counter-c])	//Mitjana de n mostres
			{
				c++;
			}
			float mitjana = suma/samples;
			printf("Mitjana últimes %d mostres : %0.1f ºC\n", samples, mitjana);	//Mostrar per pantalla la mitjana
        }
        
        if (last_temp > maxValue){		//Comprovar si hi ha un nou maxim
			maxValue = last_temp;
			printf("Nova temperatura màxima : %0.1f ºC\n", maxValue);		//Mostrar per pantalla el nou maxim
		}
		
		if (last_temp < minValue){		//Comprovar si hi ha un nou minim
			minValue = last_temp;
			printf("Nova temperatura mínima : %0.1f ºC\n", minValue);		//Mostrar per pantalla el nou minim
		}
        
        if (counter == 99) {	//Reset de la posicio de l'array circular (0-99)
			counter = 0;
		}
		else {
			counter++;			//Incrementar el contador de posicio de l'array circular
		}
        main_counter++;			//Incrementar el contador de lectures totals
        
		sleep(temps_mostreig_real-1);	//adormim el programa fins el següent cicle (segons el temps de mostreig menys el temps que ha esta adormit)
}
	
	TancarSerie(fd);	//Tanquem la comunicacio serie
	
	return 0;
}

void initialize (int *temps_mostreig_p, int *temps_mostreig_real, char *missatge)	//Funcio d'inicialitzacio del programa (posada en marxa)
{
	int temps_mostreig;
	temps_mostreig_p = &temps_mostreig;
	printf("Establir nou temps de mostreig (1-20s): ");				//Demanem temps de mostreig per treballar
	scanf("%d", &temps_mostreig);
	while ((temps_mostreig > 20) || (temps_mostreig < 1)) {			//Comprovem que esta dins dels limits demanats
		printf("Temps erroni, torni a introduir un nou valor\n");
		printf("Establir nou temps de mostreig (1-20s): ");
		scanf("%d", &temps_mostreig);
	}
		
	*temps_mostreig_real = temps_mostreig;		//Creem una variable de temps real (el demanat)
	
	if (temps_mostreig >= 2){
		temps_mostreig = temps_mostreig/2;		//Enviem a l'Arduino la meitat per asegurar-nos tenir una nova mostra en cada cicle
	}

	//Preparem el missatge a enviar
	char value[2];
	itoa(temps_mostreig, value);		//Convertim el int a char per enviar-lo
	if (temps_mostreig < 10){			//Afegim un 0 si <10 per complir el protocol
		char temp[2];
		strcpy(temp, value);
		strcpy(value, "0");
		strcat(value, temp);
	}
	printf("Temps mostreig: %s\n", value);
	char missatge1 [10] = "AM1";
	strcpy(missatge, missatge1);
	strcat(missatge, value);
	strcat(missatge, "Z\n"); //Comanda sencera
}

/* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
 
 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
