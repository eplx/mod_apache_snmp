/*
	Copyright 2004 Esteban Pizzini

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

int send_http_to_local (char* ip, char* port, char* location)
{
	int sockfd;
	char caracter;
	char linea_send [512];
	char linea_recv [512];
	int rcv_bytes;

// conexion
	struct sockaddr_in serv_addr;

	bzero( (char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);	// direccion IP
	serv_addr.sin_port = htons(port);		// puerto

	/* Opens TCP socket */
	if ( ( sockfd = socket( AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		perror( "client: socket() Error\n");
		return -1;
	}

	/* Connects */
	if ( connect (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 )
	{
		close(sockfd); // close connection
		perror("client: connect() Error \n");
		return -1;
	}


//	printf("Conectado...\n");

	strcpy(linea_recv,"");
	strcpy(linea_send,"GET /test-loc");

		if ( strlen(linea_send) > 0)
		{
			//printf("Send.. %s",linea_send);
			send( sockfd, linea_send, MAX_SEND, 0);
			strcpy(linea_recv,"");
			rcv_bytes = recv( sockfd, linea_recv, MAX_RECEIVE , 0);

			if (rcv_bytes > 0)
				{
					linea_recv[rcv_bytes] = '\0';
		 			if (strcmp(linea_recv,"OK")==0)
		 				printf("OK");
		 				return 1;	// 	OK
		 			else
		 			{
		 				printf("Error..");
		 				return 0;	//	error
		 			}
				}
			else
			{
				printf("Sin recepcion..");
				return 0;	//	error
			}
		}
		else
			return 0;	//	error
}