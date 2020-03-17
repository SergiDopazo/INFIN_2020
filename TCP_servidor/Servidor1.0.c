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

static void selector(char msg[256]);
static void marcha(char msg[256]);
static void ultimate(char msg[256]);
static void max(char msg[256]);
static void min(char msg[256]);
static void reset(char msg[256]);
static void contador(char msg[256]);
static void errorIns(char msg[256]);

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

	/*Preparar l'adre�a local*/
	sockAddrSize=sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Crear un socket*/
	sFd=socket(AF_INET, SOCK_STREAM, 0);
	
	/*Nominalitzar el socket*/
	result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	
	/*Crear una cua per les peticions de connexi�*/
	result = listen(sFd, SERVER_MAX_CONNECTIONS);
	
	/*Bucle s'acceptaci� de connexions*/
	while(1){
		printf("\nServidor esperant connexions\n");

		/*Esperar conexi�. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexi�n acceptada del client: adre�a %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		memset( buffer, 0, 256 );
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);

		/*Escollir resposta*/
		missatge=selector(buffer);
				
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
			errorIns(msg);
	}
}

void marcha(){
	if (msg[0]=='{'&&msg[6]=='}'&&msg[7]==0){
		if (msg[2]!='0'&&msg[2]!='1'){
			missatge="{M2}";
		}
	
	}
}

void ultimate(){
	oldestValue=
	if (msg=="{U}"){
		
		
		
	else
		missatge="{U1}";
	}
	
}

