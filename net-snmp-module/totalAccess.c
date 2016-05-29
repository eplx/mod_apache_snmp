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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/watcher.h>
#include "totalAccess.h"
#include "module_init.h"


/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */

static unsigned long    totalAccess = 0;       		// accesos totales del Apache
extern char		serverTmpDir_str[MY_MAX_LEN];	// ubicacion archivo de intercambio
extern int 		agentHttpAccess;	 	// total accesos HTTP del agente
extern char 		agentHttpAddress_str[MY_MAX_LEN];

/*
 * our initialization routine, automatically called by the agent
 * (to get called, the function name must match init_FILENAME())
 */
void
init_totalAccess(void)
{
    static oid      totalAccess_oid[] =
        { MOD_SNMP_OID 2, 2 };

    /*
     * a debugging statement.  Run the agent with -DtotalAccess to see
     * the output of this debugging statement.
     */
    DEBUGMSGTL(("totalAccess", "Initializing the totalAccess module\n"));


    /*
     * the line below registers our variables defined above as
     * accessible and makes them writable.  A read only version of any
     * of these registrations would merely call
     * register_read_only_int_instance() instead.  The functions
     * called below should be consistent with your MIB, however.
     *
     * If you wanted a callback when the value was retrieved or set
     * (even though the details of doing this are handled for you),
     * you could change the NULL pointer below to a valid handler
     * function.
     */
    DEBUGMSGTL(("totalAccess",
                "Initializing totalAccess scalar integer.  Default value = %d\n",
                totalAccess));

    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "totalAccess",
            handler_totalAccess,
            totalAccess_oid,
            OID_LENGTH(totalAccess_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &totalAccess, sizeof(totalAccess),
            ASN_COUNTER, WATCHER_MAX_SIZE)
        );
    DEBUGMSGTL(("totalAccess", "Done initalizing totalAccess module\n"));
}

int
handler_totalAccess(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {

    case MODE_GET:
	// 22/07/04 - EP - don´t need call check_estado here
	//	if (check_estado())		// check Apache status
	//	{
			//int x;		// not used
	//DEBUGMSGTL(("totalAccess", "send_http_get -->\n"));
	if (send_http_get("/ap2_snmp"))
	{
		FILE *fp;		// exchange file
    		//DEBUGMSGTL(("totalAccess","send_http_get.  valor= %d\n",x));
    		 if ((fp = fopen(serverTmpDir_str,"r")) == NULL)
			{
				DEBUGMSGTL(("totalAccess", "cannot open temporal file."));
			}
		 else
		 	{
				DEBUGMSGTL(("totalAccess", "Reading data from temporal file."));
				rewind(fp);
			 	fread(&totalAccess,sizeof(unsigned long),1,fp);
				fclose(fp);
				DEBUGMSGTL(("totalAccess","totalAccess = %i - agentHttpAccess= %i\n",totalAccess,agentHttpAccess));
				/*
					21/07/04 - EP - totalAccess returns only client's http requests (not count snmp agent HTTP requests)
				*/
				totalAccess = totalAccess - agentHttpAccess;
		    		snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
			                        	(u_char *)
			                        	&totalAccess
			                        	,
			                        	sizeof(totalAccess)
			                        	);
		 	}
	}
        break;
    case MODE_SET_RESERVE1:
	break;
    case MODE_SET_RESERVE2:
	break;
    case MODE_SET_FREE:
	break;
    case MODE_SET_ACTION:
	break;
    case MODE_SET_COMMIT:
        break;
    case MODE_SET_UNDO:
        break;

    default:
        /*
         * we should never get here, so this is a really bad error
         */
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

int send_http_get (char* uri)
{
	int sockfd;
	char caracter;
	char linea_send [MAX_SEND];
	char linea_recv [MAX_RECEIVE];
	int rcv_bytes;
	char* http_port;
	char agentHttpAddress_str_temp[MY_MAX_LEN];

// conexion
	struct sockaddr_in serv_addr;

	bzero( (char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	if (strlen(agentHttpAddress_str) == 0)
	{
		serv_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);	// IP Address
		serv_addr.sin_port = htons(DEFAULT_PORT);		// Port
	}
	else
	{
		strcpy(agentHttpAddress_str_temp,agentHttpAddress_str); // Temporal string
		http_port = strtok(agentHttpAddress_str_temp,":");
		http_port = strtok('\0',":");
		serv_addr.sin_addr.s_addr = inet_addr(agentHttpAddress_str_temp);	// Ip Address
		serv_addr.sin_port = htons(atol(http_port));		// Port
		DEBUGMSGTL(("totalAccess","send_http_get.  address= %s - port= %s\n",agentHttpAddress_str_temp,http_port));
	}


	/* Opens TCP socket */
	if ( ( sockfd = socket( AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		close(sockfd);	// close connection
		perror( "client: socket() Error\n");
		DEBUGMSGTL(("totalAccess", "send_http_get --> Socket Error\n"));
		return -1;
	}

	/* Connects */
	if ( connect (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 )
	{
		perror("client: connect() Error \n");
		DEBUGMSGTL(("totalAccess", "send_http_get --> Connect Error\n"));
		return -1;
	}

	//	printf("Conectado...\n");

	strcpy(linea_recv,"");
	sprintf(linea_send,"GET %s \r\n",uri);
	DEBUGMSGTL(("totalAccess", "send_http_get --> send GET %s \n",uri));
	//strcpy(linea_send,"GET /ap2_snmp \r\n");

		if ( strlen(linea_send) > 0)
		{
			//printf("Send.. %s",linea_send);
			send( sockfd, linea_send, MAX_SEND, 0);
			strcpy(linea_recv,"");
			rcv_bytes = recv( sockfd, linea_recv, MAX_RECEIVE , 0);

			if (rcv_bytes > 0)
				{
					linea_recv[rcv_bytes] = '\0';
				DEBUGMSGTL(("totalAccess", "send_http_get --> response %s \n",linea_recv));
				if (strcmp(linea_recv,"OK")==0)
		 			{
		 				printf("OK");
		 				DEBUGMSGTL(("totalAccess", "send_http_get --> return Ok\n"));
		 				close(sockfd);
		 				agentHttpAccess++;  // + HTTP request generated by SNMP agent (to check status)
		 				return 1;	// 	OK
		 			}
		 			else
		 			{
		 				printf("AP2-SNMP: HTTP Response Error.");
		 				DEBUGMSGTL(("totalAccess", "send_http_get --> return HTTP Error\n"));
		 				close(sockfd);
		 				return 0;	//	error
		 			}
				}
			else
			{
				printf("AP2-SNMP: cannot connect (HTTP)");
				close(sockfd);
				return 0;	//	error
			}
		}
		else
		{
			close(sockfd);
			return 0;	//	error
		}
}