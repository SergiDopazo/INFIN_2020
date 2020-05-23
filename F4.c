#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/stat.h>                                                     
#include <fcntl.h>                                                        
#include <termios.h>                                                      
#include <strings.h>
#include <sys/ioctl.h>   
#include <stdbool.h>  

//Arduino
#define BAUDRATE B9600                                                
#define MODEMDEVICE "/dev/ttyACM0"        //Conexió directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 /* POSIX compliant source */    

//TCP/IP
#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4
#define REQUEST_MSG_SIZE	1024

//Inicialització funcions
static void selector();
static void marcha();
static void oldest();
static void max();
static void min();
static void reset();
static void contador();
int ConfigurarSerie();
void TancarSerie();
void initialize();
void itoa();
void reverse();

//Config serie
	struct termios oldtio,newtio; 

//Variables lectura
	int state=0; //marxa o parada
	int temps=0; //temps entre mostres
	int samples=0; //número de mostres per fer mitjana

//Creació array de mostres
	float array[100];
	float maxValue;
	float minValue;
	
	int counterR=0; //contador de dades array circular
	int counterW=0; //contador de dades array circular

//Creació threads
	pthread_t thread;
	pthread_mutex_t lock;

void* codi_fill(void* parametre){ //codi thread fill
   	int fd, res;
   	//int i = 0;
	char buf[255];
	char missatge[255];
	float array_temp [100];			//Array temp
	int main_counter = 1; 			//Contador de lectures total
	int counter_temp = 0;			//Posicio dins l'array temp (lectures Arduino)
	maxValue = 0;			//Valor maxim
	minValue = 70;			//Valor minim
	
	int marcha = 0;
	int state_check = 0; 
	//Comprovació ordre de marxa (donada pel client
	do {
		if (state_check != state){
			state_check = state;
			if (state_check == 1){
				marcha = 1;
			}
			else{
				marcha = 0;
			}
		}
	} while(marcha==0);
	
	fd = ConfigurarSerie();
	
	initialize(&missatge);	//Funcio inicial de posada en marxa

	res = write(fd,missatge,strlen(missatge));	//Enviem missatge

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

	/*printf("Enviats %d bytes: ",res);		//Ensenyem per pantalla el missatge
	for (i = 0; i < res; i++)
	{
		printf("%c",missatge[i]);
	}
	printf("\n");
	*/
	
	//Rebem
	res = read(fd,buf,1);
	res = res + read(fd,buf+1,1);
	res = res + read(fd,buf+2,1);
	res = res + read(fd,buf+3,1);
	res = res + read(fd,buf+4,1);
	res = res + read(fd,buf+5,1);

	/*
	printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
	for (i = 0; i < res; i++)
	{
		printf("%c",buf[i]);			//Ensenyem la resposta per pantalla
	}
	printf("\n");
	*/
	
	while (1)		//Comencem el loop infinit on demanem mostres segons temps de mostreig donat
	{
		if (state_check != state){ //Comprovació ordre de marxa
			state_check = state;
			if (state_check == 1){
				marcha = 1; //Nova marxa, enviem nous valors a l'Arduino (necessitat de previa parada)
				
				initialize(&missatge);	//Funcio de posada en marxa

				res = write(fd,missatge,strlen(missatge));	//Enviem missatge
			
				if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
				
				/*
				printf("Enviats %d bytes: ",res);		//Ensenyem per pantalla el missatge
				for (i = 0; i < res; i++)
				{
					printf("%c",missatge[i]);
				}
				printf("\n");
				*/
				
				//Rebem
				res = read(fd,buf,1);
				res = res + read(fd,buf+1,1);
				res = res + read(fd,buf+2,1);
				res = res + read(fd,buf+3,1);
				res = res + read(fd,buf+4,1);
				res = res + read(fd,buf+5,1);
				
				/*
				printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
				for (i = 0; i < res; i++)
				{
					printf("%c",buf[i]);			//Ensenyem la resposta per pantalla
				}
				printf("\n");
				*/

			}
			else{
				marcha = 0;	//Enviem ordre de parada
				strcpy(missatge,"AM000Z\n");
				res = write(fd,missatge,strlen(missatge));	//Enviem missatge
			
				if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
			
				/*
				printf("Enviats %d bytes: ",res);		//Ensenyem per pantalla el missatge
				for (i = 0; i < res; i++)
				{
					printf("%c",missatge[i]);
				}
				printf("\n");
				*/
				
				//Rebem
				res = read(fd,buf,1);
				res = res + read(fd,buf+1,1);
				res = res + read(fd,buf+2,1);
				res = res + read(fd,buf+3,1);
				res = res + read(fd,buf+4,1);
				res = res + read(fd,buf+5,1);
				
				/*
				printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
				for (i = 0; i < res; i++)
				{
					printf("%c",buf[i]);			//Ensenyem la resposta per pantalla
				}
				printf("\n");
				*/
				
			}
		}
		if (marcha == 1){	//Lectures quan esta en marxa
			printf("\n________________________________________________________________________________\n\n");	//Separador
			char missatge2 [10] = "ACZ\n";	//Comanda ultima mostra
			strcpy(missatge,missatge2);
	
			res = write(fd,missatge,strlen(missatge));	//Enviar missatge
	
			if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

			/*
			printf("Enviats %d bytes: ",res);
			for (i = 0; i < res; i++)
			{
				printf("%c",missatge[i]);	//Ensenyem per pantalla el missatge
			}
			printf("\n");
			*/
			
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
			
			/*
			printf("Rebuts %d bytes: ",res);	//Mirem quina ha estat la resposta al buffer
			for (i = 0; i < res; i++)
			{	
				printf("%c",buf[i]);			//Ensenyem per pantalla la resposta
			}
			printf("\n");
			*/
			      
	        float last_value = 0;
	        last_value = (buf[3]-48)*1000+(buf[4]-48)*100+(buf[5]-48)*10+(buf[6]-48)*1;	//Guardem el valor de la ulitma mostra (0-1023)
			
			//Procés del parpadeix
	        char missatge3 [10] = "AS131Z\n";	//Comanda per encendre el led (sortida digital 13 HIGH)
			strcpy(missatge,missatge3);
			
			res = write(fd,missatge,strlen(missatge));		//Enviar missatge
	
			if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
			
			/*
			printf("Enviats %d bytes: ",res);	
			for (i = 0; i < res; i++)
			{
				printf("%c",missatge[i]);		//Ensenyem missatge per pantalla
			}
			printf("\n");
			*/
	
			//Rebem
			res = read(fd,buf,1);
			res = res + read(fd,buf+1,1);
			res = res + read(fd,buf+2,1);
			res = res + read(fd,buf+3,1);
			res = res + read(fd,buf+4,1);
			res = res + read(fd,buf+5,1);
			
			/*
			printf("Rebuts %d bytes: ",res);	//Mirem la resposta al buffer
			for (i = 0; i < res; i++)
			{
				printf("%c",buf[i]);			//Ensenyem resposta per pantalla
			}
			printf("\n");
			*/
			
			sleep(1);	//adormim el programa per observar el led
			
			char missatge4 [10] = "AS130Z\n";	//Comanda per apagar el led (sortida digital 13 LOW)
			strcpy(missatge,missatge4);
			
			res = write(fd,missatge,strlen(missatge));	//Enviar missatge
	
			if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
			
			/*
			printf("Enviats %d bytes: ",res);
			for (i = 0; i < res; i++)
			{
				printf("%c",missatge[i]);		//Mostrem per pantalla el missatge
			}
			printf("\n");
			*/
			
			//Rebem
			res = read(fd,buf,1);
			res = res + read(fd,buf+1,1);
			res = res + read(fd,buf+2,1);
			res = res + read(fd,buf+3,1);
			res = res + read(fd,buf+4,1);
			res = res + read(fd,buf+5,1);
			
			/*
			printf("Rebuts %d bytes: ",res);	//Mirem la resposta al buffer
			for (i = 0; i < res; i++)
			{	
				printf("%c",buf[i]);			//Ensenyem per pantalla la resposta
			}	
	        printf("\n");
	        */
	        
	        float last_temp = 0;
			int temp_inf = 0, temp_sup = 70;
			int rang_temp = temp_sup - temp_inf;
			last_temp = (last_value/1023)*rang_temp;							//Conversio a graus celsius
			printf("Última temperatura : %0.1f ºC\n", last_temp);				//Mostrar per pantalla la ultima temperatura registrada
			printf("Número de lectures realitzades : %d\n", main_counter);		//Mostrar per pantalla el nombre de mostres realitzades totals
			
			array_temp [counter_temp] = last_temp;	//Guardar la lectura dins l'array circular
	
			if (main_counter >= samples){		//Mitjana quan hi hagi el nombre demanat de mostres
				float suma = last_temp;
				for (int c = 1; c < samples; c++)	//Mitjana de n mostres
				{
					suma = suma + array_temp [counter_temp-c];
				}
				
				pthread_mutex_lock(&lock);	//Bloquejem per editar el valor del array i altres valors compartits
				
				array[counterW] = suma/samples;
				printf("Mitjana últimes %d mostres : %0.1f ºC\n", samples, array[counterW]);	//Mostrar per pantalla la mitjana
	        
		        if (array[counterW] > maxValue){		//Comprovar si hi ha un nou maxim
					maxValue = array[counterW];
					printf("Nova temperatura màxima : %0.1f ºC\n", maxValue);		//Mostrar per pantalla el nou maxim
				}
				
				if (array[counterW] < minValue){		//Comprovar si hi ha un nou minim
					minValue = array[counterW];
					printf("Nova temperatura mínima : %0.1f ºC\n", minValue);		//Mostrar per pantalla el nou minim
				}
				
				if (counterW == 99) {	//Reset de la posicio de l'array temp (0-99)
					counterW = 0;
				}
				else {
				counterW++;			//Incrementar el contador de posicio de l'array 
				}
				pthread_mutex_unlock(&lock);
			}
			
	        if (counter_temp == 99) {	//Reset de la posicio de l'array temp (0-9)
				counter_temp = 0;
			}
			else {
				counter_temp++;			//Incrementar el contador de posicio de l'array temp
			}
	        main_counter++;			//Incrementar el contador de lectures totals
	        
			sleep(temps-1);	//adormim el programa fins el següent cicle (segons el temps de mostreig menys el temps que ha esta adormit)
		}
}
	
	TancarSerie(fd);	//Tanquem la comunicacio serie 	
   	   	
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]){
	
	pthread_create(&thread, NULL, codi_fill, 0); //Es crea el thread fill
	if (pthread_mutex_init(&lock, NULL)!=0){
		printf("\n mutex init failed\n");
		return 1;
	}	
	
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int		sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char		buffer[256];
	char missatge[256] = ""; 
	
	/*Preparar l'adreça local*/
	sockAddrSize=sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Crear un socket*/
	sFd=socket(AF_INET, SOCK_STREAM, 0);

	/*Nominalitzar el socket*/
	result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);

	/*Crear una cua per les peticions de connexió*/
	result = listen(sFd, SERVER_MAX_CONNECTIONS);

	/*Bucle s'acceptació de connexions*/
	while(1){
		printf("\nServidor esperant connexions\n");

		/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		memset( buffer, 0, 256 );
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);

		/*Escollir resposta*/
		selector(buffer, &missatge);

		/*Enviar*/
		strcpy(buffer,missatge); //Copiar missatge a buffer
		result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
		printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);

		/*Tancar el socket fill*/
		result = close(newFd);
	}
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&lock);
}

void selector(char msg[256], char *missatge){ //Seleccionar funció segons acció dessitjada
	switch(msg[1]){ //Comprovar segon valor (Informació de la lletra)
		case 'M':
			marcha(msg);
			break;
		case 'U':
			oldest(msg);
			break;
		case 'X':
			max(msg);
			break;
		case 'Y':
			min(msg);
			break;
		case 'R':
			reset(msg);
			break;
		case 'B':
			contador(msg);
			break;
		default:
			strcpy(missatge,"{E2}");//Error de paràmetre
	}
}
	
void marcha(char msg[256], char *missatge){

	int msgOK=0;
	int desens=0;
	int units=0;
	//Comprovació errors
	if (msg[0]=='{'&&msg[6]=='}'&&msg[7]==0){
		if (msg[2]=='1'){  
			if (msg[3]>'2'||msg[3]<'0'){
	      		strcpy(missatge,"{M2}");
	        }
			else if (msg[4]>'9'||msg[4]<'0'){
	        	strcpy(missatge,"{M2}");
	    	}
			else if (msg[3]=='2'&&msg[4]>'0'){
				strcpy(missatge,"{M2}");
			}
			else if (msg[5]>'9'||msg[5]<'1'){
	        	strcpy(missatge,"{M2}");
			}
			else{
				strcpy(missatge,"{M0}");   //Missatge correcte
				msgOK=1;
			}
		}
		else if (msg[2]=='0'){
			if (msg[3]!='0'){
	      		strcpy(missatge,"{M2}");
	        }
			else if (msg[4]!='0'){
	        	strcpy(missatge,"{M2}");
	    	}
			else if (msg[5]!='0'){
	        	strcpy(missatge,"{M2}");
			}
			else{
				strcpy(missatge,"{M0}");   //Missatge correcte
				msgOK=1;
			}
		}
	}
	else{ //Error de protocol
		strcpy(missatge,"{M1}");
	}

    if (msgOK==1){ //Assignació valors per a la lectura
    	pthread_mutex_lock(&lock);	//Bloquejem la lectura de valors d'entrada del client
    	
    	state=msg[2]-'0';
    	desens=(msg[3]-'0')*10;
    	units=msg[4]-'0';
    	temps=desens+units;
    	samples=msg[5]-'0';
    	
    	pthread_mutex_unlock(&lock);	//Desbloquejem
	}	
}

void oldest(char msg[256], char *missatge){
	float oldValue = array[counterR];
	char value[5]="";
	char missatge1[4]="{U0";
		
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){ 
		strcpy(missatge,missatge1);
		gcvt(oldValue,4,value); //Convertir float a string
		int l=strlen(value);
		if (l<5){
			for (int i=5-l; i>0; i--){
				char temp[]="";
				strcpy(temp,value);
				strcpy(value,"0");
				strcat(value,temp);
			}
		}
		strcat(missatge,value);
		strcat(missatge,"}"); //Concatenar missatge d'acord al protocol
		counterR++; //Suma counter per "eliminar" la mostra
		if(counterR==100){
			counterR=0;
		}
	}
	else{
		strcpy(missatge,"{U1}"); //Error de protocol
	}
}

void max(char msg[256], char *missatge){	
	char value[5];
	char missatge1[]="{X0";
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){
		strcpy(missatge,missatge1);
		gcvt(maxValue,4,value); //Convertir float a string
		int l=strlen(value);
		if (l<5){
			for (int i=5-l; i>0; i--){
				char temp[]="";
				strcpy(temp,value);
				strcpy(value,"0");
				strcat(value,temp);
			}
		}
		strcat(missatge,value);
		strcat(missatge,"}"); //Concatenar missatge d'acord al protocol
	}
	else{
		strcpy(missatge,"{X1}");
	}
}

void min(char msg[256], char *missatge){
	char value[5];
	char missatge1[]="{Y0";
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){
		strcpy(missatge,missatge1);
		if(minValue<1){
			gcvt(minValue,3,value);
		}
		else{
			gcvt(minValue,4,value); //Convertir float a string
		}
		int l=strlen(value);
		if (l<5){
			for (int i=5-l; i>0; i--){
				char temp[]="";
				strcpy(temp,value);
				strcpy(value,"0");
				strcat(value,temp);
			}
		}
		strcat(missatge,value);
		strcat(missatge,"}"); //Concatenar missatge d'acord al protocol
	}
	else{
		strcpy(missatge,"{Y1}"); //Error de protocol
	}
}

void reset(char msg[256], char *missatge){
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){
    minValue=70; //Valors per ressetejar
    maxValue=0;
    strcpy(missatge,"{R0}"); //Tot OK
	}
	else{
		strcpy(missatge,"{R1}"); //Error de protocol
	}

}

void contador(char msg[256], char *missatge){
	char value[5];
	int left;
	if (counterR<counterW){
		left = counterW - counterR;
	}
	else{
		left = 100 - (counterR - counterW);
	}
	char missatge1[10]="";
	if (left<100) { //Principi del missatge segons magnitud
		if (left<10){
			strcpy(missatge1,"{B0000");
		}
		else{
		strcpy(missatge1,"{B000");
		}
	}
	else {
		strcpy(missatge1,"{B00");
	}
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){
		strcpy(missatge,missatge1);
		itoa(left,value); //Convertir de integer a string
		strcat(missatge,value);
		strcat(missatge,"}");
	}
	else{
		strcpy(missatge,"{B1}"); //Error de protocol
	}
}

void initialize (char *missatge){	//Funcio d'inicialitzacio del programa (posada en marxa)

	int temps_mostreig=temps;
		
	if (temps_mostreig >= 2){
		temps_mostreig = temps_mostreig/2;		//Enviem a l'Arduino la meitat per asegurar-nos tenir una nova mostra en cada cicle
	}

	//Preparem el missatge a enviar
	char missatge1 [10] = "AM1";
	strcpy(missatge, missatge1);
	
	char value[2];
	itoa(temps_mostreig, value);		//Convertim el int a char per enviar-lo
	if (temps_mostreig < 10){			//Afegim un 0 si <10 per complir el protocol
		char temp[2];
		strcpy(temp, value);
		strcpy(value, "0");
		strcat(value, temp);
	}
	//printf("Temps mostreig: %s\n", value);
	
	strcat(missatge, value);
	strcat(missatge, "Z\n"); //Comanda sencera
}

/* itoa:  convert n to characters in s */
 void itoa(int n, char s[]){
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
 void reverse(char s[]){
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

int	ConfigurarSerie(void){	//Obrir comunicacio serie
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

void TancarSerie(int fd){ //Tancar comunicacio serie

	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
