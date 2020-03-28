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


#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		5001




char comunicar(char* msg){ //Subrutina de comunicaciones.
	
	
//Define las variables necesarias para poder establecer una conexión con el servidor.

	struct sockaddr_in	serverAddr;
	char	    serverName[] = "88.6.4.147"; //Adreça IP on est� el servidor
	int			sockAddrSize;
	int			sFd;
	int			mlen;
	int 		result;
	char		buffer[256];
	char		recibo[256];
	
	/*Crear el socket*/
	sFd=socket(AF_INET,SOCK_STREAM,0);

	/*Construir l'adreça*/
	sockAddrSize = sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons (SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr(serverName);

	/*Conexió*/
	result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	if (result < 0)
	{
		printf("Error en establir la connexió\n");
		exit(-1);
	}
	printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	/*Enviar*/
	strcpy(buffer,msg); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("\nMensaje enviado al servidor (bytes %d): %s",	result, msg);

	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("\nRespuesta del servidor (bytes %d): %s\n",	result, buffer);

	strcpy(recibo,buffer);
	//printf("\n\n\nRECIBO: %s\n\n\n", recibo);

//Una vez que se recibe el mensaje, contenido en el buffer, se separa para presentrlo al usuario.

	printf("\n================================================================================");	
	
	if(recibo[2]=='0'){ //Si el codigo de error es 0 (no hay error), se muestra la información al usuario.
		switch(recibo[1]){
			case 'M':				
				printf("\nRegistrador en marcha\n"); //Confirmación de marcha.
				break;
		
			case 'U':		
				printf("\nTemperatura más antigua registrada (ºC): "); //Se imprime la temperatura más antigua.
				printf("%c",recibo[3]);
				printf("%c",recibo[4]);
				printf("%c",recibo[5]);
				printf("%c",recibo[6]); 
				printf("%c",recibo[7]);
				printf("\n");
				break;
		
			case 'X':
				printf("\nTemperatura máxima registrada (ºC): "); //Se imprime el valor máximo de temperatura. (Recibido como caracteres).
				printf("%c",recibo[3]);
				printf("%c",recibo[4]);
				printf("%c",recibo[5]);
				printf("%c",recibo[6]); 
				printf("%c",recibo[7]);
				printf("\n");
				break;
		
			case 'Y':
				printf("\nTemperatura mínima registrada (ºC): "); //Se imprime el valor mínimo de temperatura.
				printf("%c",recibo[3]);
				printf("%c",recibo[4]);
				printf("%c",recibo[5]);
				printf("%c",recibo[6]); 
				printf("%c",recibo[7]);
				printf("\n");
			break;
		
			case 'R':
				printf("\nValores máximo y mínimo reseteados\n"); //Confirmación de reinicio de valores máximo y mínimo.
			break;
		
			case 'B':
				printf("\nMuestras guardadas: "); //Se imprime la cuenta de valores almacenados recibida como caracteres desde el servidor.
				printf("%c",recibo[3]);
				printf("%c",recibo[4]);
				printf("%c",recibo[5]);
				printf("%c",recibo[6]); 
				printf("\n");
			break;	
		
		
		}
	}
	
	else if(recibo[2]=='1'){ //Si el codigo de error es 1, se imprime el texto del error correspondiente.
		printf("\nError 1: Error de protocolo\n"); }
	else if(recibo[2]=='2'){ //Si es 2, se imprime el mensaje que corresponde.
		printf("\nError 2: Error en parámetros\n"); }

	printf("================================================================================");

	/*Tancar el socket*/
	close(sFd);

	return 0;
	}



//ImprimirMenu

void ImprimirMenu(void)
{
	//printf("\e[1;1H\e[2J");
  	printf("\n\nREGISTRADOR DE TEMPERATURA");
	printf("\n\nMenú principal\n");
	printf("--------------------\n");
	printf("1: Muestra más antigua\n");
	printf("2: Muestra máxima\n");
	printf("3: Muestra mínima\n");
	printf("4: Reset máximo y mínimo\n");
	printf("5: Número muestras almacenadas\n");
	printf("6: Marcha / Restablecer parámetros\n");
	printf("7: Paro\n");
	printf("s: Terminar programa\n");
	printf("--------------------\n");
	printf("\nIntroduzca opción deseada: ");
}


//Función main. Menú de selección.

int main(int argc, char **argv)                                                               
{   
	//Definición de variables para el funcionamiento del menú.
	
	char missatge[8]=""; 
	char input;
	char tiempo[2]="";
	char muestras[1]="";
	char condiciones[3]="";


	ImprimirMenu();     //Se llama a la función que imprime el texto del menú.                        
	input = getchar();

	while (input != 's') //Presionando la s se puede abortar la ejecución del programa.
	{
		switch (input) //Se e la tecla que ha pulsado el usuario. A continuación se muestran las diferentes opciones.
		{
			case '1':
				printf("\n\n\nSolicitado valor más antiguo al servidor...\n");
				strcpy(missatge,"{U}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);
				comunicar(missatge);
				ImprimirMenu();                          
				break;
			case '2':
				printf("\n\n\nSolicitado valor máximo al servidor...\n");
				strcpy(missatge,"{X}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);	
				comunicar(missatge);
				ImprimirMenu();                             
				break;
			case '3':
				printf("\n\n\nSolicitado valor mínimo al servidor...\n");	
				strcpy(missatge,"{Y}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);	
				comunicar(missatge);
				ImprimirMenu();                             
				break;
			case '4':
				printf("\n\n\nReiniciando valores máximo y mínimo...\n");	
				strcpy(missatge,"{R}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);	
				comunicar(missatge);
				ImprimirMenu();                             
				break;
			case '5':
				printf("\n\n\nSolicitado número muestras almacenadas al servidor...\n");	
				strcpy(missatge,"{B}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);	
				comunicar(missatge);
				ImprimirMenu();                             
				break;
			
			case '6':				 //Este caso es diferente, muestra un submenú en el que configurar los valores de tiempo de muestreo y promedio.
				
				printf("\n\n  Introduzca tiempo de muestreo (s): ");
				scanf("%s",tiempo);
				
				if (strlen(tiempo)<2){
					
					tiempo[1]=tiempo[0];
					tiempo[0]='0';
					tiempo[2]='\0';
				}
				
				strcat(condiciones, tiempo);
				printf("  Introduzca número muestras promedio: ");
				scanf("%s",muestras);
				strcat(condiciones, muestras);
			
				
				missatge[0]='{';
				missatge[1]='M';
				missatge[2]='1';
				missatge[3]=condiciones[0];
				missatge[4]=condiciones[1];
				missatge[5]=condiciones[2];
				missatge[6]='}';
				
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n", missatge);
				printf("\n\n\nIniciando el registro...\n");
				comunicar(missatge);
				ImprimirMenu();
				
				break;
			
			case '7':
				printf("\n\n\nParando el registro...\n");
				strcpy(missatge,"{M0000}");
				//printf("\n\n\nEl mensaje enviado es: %s\n\n\n",missatge);		
				comunicar(missatge);
				ImprimirMenu();                             
				break;			
				
			case 0x0a:
				break;
			default: //Se ejecuta en caso de que se pulse una tecla no contemplada en el menú.
				printf("\nOpción incorrecta\n");	
				printf("\nIntroduzca una opción válida\n");
				ImprimirMenu();
				break;
		}

		input = getchar();

	}
	
	return 0;
}


