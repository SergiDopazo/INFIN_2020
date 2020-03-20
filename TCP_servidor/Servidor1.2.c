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
#include <string.h>

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
static void selector(char msg[256]);
static void marcha(char msg[256]);
static void oldest(char msg[256]);
static void max(char msg[256]);
static void min(char msg[256]);
static void reset(char msg[256]);
static void contador(char msg[256]);
static void errorIns(char msg[256]);

//Variables lectura
	int estate=0; //marxa o parada
	int time=0; //temps entre mostres
	int samples=0; //número de mostres

//Creació array de mostres
	float array[100];
	float maxValue;
	float minValue;
	float oldValue;
	float counter=0; //contador de dades

int main(int argc, char *argv[])
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int			sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char		buffer[256];
	char		missatge[256];

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

void selector(){
	switch(msg[1]){
		case 'M':
			marcha(msg);
			break;
		case 'U':
			ultimate(msg);
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
			missatge="{E2}"; //Error de paràmetre
	}
}

void marcha(){
    
	int msgOK=0;
	int desens=0;
	int units=0;
	//Comprovació errors
	if (msg[0]=='{'&&msg[6]=='}'&&msg[7]==0){	 
		if (msg[2]!='0'&&msg[2]!='1'){
			missatge="{M2}"; //Error de paràmetres
        }
		else if (msg[2]!='1'&&msg[2]!='0'){
          	missatge="{M2}"; 
        }
		else if (msg[3]>'9'||msg[3]<'0'){
      		missatge="{M2}"; 
        }
		else if (msg[4]>'9'||msg[4]<'0'){
        	missatge="{M2}"; 
    	}
		else if (msg[5]>'9'||msg[5]<'1'){
        	missatge="{M2}"; 
		}
		else{
			missatge="{M0}";    //Missatge correcte
			msgOK=1;
		}
	}
	else{ //Error de protocol
		missatge="{M1}";
	}
	
    if (msgOK=1){ //Assignació valors per a la lectura
    	estate=msg[2]-'0';
    	desens=(msg[3]-'0')*10;
    	units=msg[4]-'0';
    	time=desens+units;
    	samples=msg[5]-'0';	 	
	}
}

void oldest(){
	oldValue = array[counter+1];
	char value[5];
	if (msg=="{U}"){
		gcvt(oldValue,5,value);
		missatge="{U0"
		strcat(missatge,value);
		strcat(missatge,"}");
	else
		missatge="{U1}";
	}
	
}


void max(){
	maxValue = array[0];
	char value[5];
  	for (int c = 1; c < size; c++) {
    	if (array[c] > maxValue) {
       		maxValue  = array[c];
    	}
  	}
	if (msg=="{X}"){
        gcvt(maxValue,5,value);
		missatge="{X0"
		strcat(missatge,value);
		strcat(missatge,"}");  
	else
		missatge="{X1}";
	}
	
}

void min(){
	minValue = array[0];
	char value[5];
	for (int c = 1; c < size; c++) {
    	if (array[c] < minValue) {
       		minValue  = array[c];
    	}
  	}
	if (msg=="{Y}"){
		gcvt(minValue,5,value);
		missatge="{Y0"
		strcat(missatge,value);
		strcat(missatge,"}");     
	else
		missatge="{Y1}";
	}
	
}


void reset(){
	if (msg=="{R}"){
        minValue=100000;
        maxValue=0;
        missatge="{R0}";
	else
		missatge="{R1}";
	}
	
}

void contador(){
	char value[4];
	if (msg=="{B}"){
        gcvt(counter,4,value);
		missatge="{B0"
		strcat(missatge,value);
		strcat(missatge,"}");
	else
		missatge="{B1}";
	}
	
}

