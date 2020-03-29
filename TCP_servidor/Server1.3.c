/***************************************************************************
                          main.c  -  server
                             -------------------
    begin                : lun feb  4 15:30:41 CET 2002
    copyright            : (C) 2002 by A. Moreno
    copyright            : (C) 2020 by A. Fontquerni
    email                : amoreno@euss.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include <time.h>



#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024


/************************
*
*
* tcpServidor
*
*
*/

//Inicialització funcions
static void selector();
static void marcha();
static void oldest();
static void max();
static void min();
static void reset();
static void contador();
void itoa();
void reverse();

//Variables lectura
	int estate=0; //marxa o parada
	int temps=0; //temps entre mostres
	int samples=0; //número de mostres per fer mitjana

//Creació array de mostres
	float array[100];
	float maxValue;
	float minValue;
	float oldValue;
	int counter=0; //contador de dades
	
	char missatge[256]=""; 

int main(int argc, char *argv[])
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int			sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char		buffer[256];
	

	/*Generar 100 mostres*/
	srand((unsigned)time(NULL));
	for(counter=0;counter<100;counter++)
	{
			array[counter] = ((float)rand()/RAND_MAX)*1000;
			printf("(%d) %0.1f\n",counter,array[counter]); //Imprimir mostres
	}
	counter = 0;
	
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
		selector(buffer);

		/*Enviar*/
		strcpy(buffer,missatge); //Copiar missatge a buffer
		result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
		printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);

		/*Tancar el socket fill*/
		result = close(newFd);
	}
}

void selector(char msg[256]){ //Seleccionar funció segons acció dessitjada
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
	
void marcha(char msg[256]){

	int msgOK=0;
	int desens=0;
	int units=0;
	//Comprovació errors
	if (msg[0]=='{'&&msg[6]=='}'&&msg[7]==0){
		if (msg[2]!='0'&&msg[2]!='1'){
			strcpy(missatge,"{M2}"); //Error de paràmetres
        }
		else if (msg[2]!='1'&&msg[2]!='0'){
          	strcpy(missatge,"{M2}");
        }
		else if (msg[3]>'2'||msg[3]<'0'){
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
	else{ //Error de protocol
		strcpy(missatge,"{M1}");
	}

    if (msgOK==1){ //Assignació valors per a la lectura
    	estate=msg[2]-'0';
    	desens=(msg[3]-'0')*10;
    	units=msg[4]-'0';
    	temps=desens+units;
    	samples=msg[5]-'0';
	}
}

void oldest(char msg[256]){
	oldValue = array[counter];
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
		counter++; //Suma counter per "eliminar" la mostra
		if(counter==100){
		counter=0;
		}
	}
	else{
		strcpy(missatge,"{U1}"); //Error de protocol
	}
}

void max(char msg[256]){
	maxValue = array[0];
	char value[5];
	char missatge1[]="{X0";
  for (int c = 1; c < 100; c++) {
  	if (array[c] > maxValue) { //Buscar valor máxim
    	maxValue  = array[c];
    }
  }
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

void min(char msg[256]){
	minValue = array[0];
	char value[5];
	char missatge1[]="{Y0";
	for (int c = 1; c < 100; c++) { //Buscar valor mínim
    if (array[c] < minValue) {
    	minValue  = array[c];
    }
  }
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

void reset(char msg[256]){
	if (msg[0]=='{'&&msg[2]=='}'&&msg[3]==0){
    minValue=100000; //Valors per ressetejar
    maxValue=0;
    strcpy(missatge,"{R0}"); //Tot OK
	}
	else{
		strcpy(missatge,"{R1}"); //Error de protocol
	}

}

void contador(char msg[256]){
	char value[5];
	int left=(99-counter)+1; //Valors que queden per llegir
	char missatge1[10]="";
	if (left<100) { //Principi del missatge segons magnitud
		if (left<10){
			strcpy(missatge1,"{B0000");
		}
		strcpy(missatge1,"{B000");
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

