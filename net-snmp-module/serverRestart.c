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
#include "serverRestart.h"
#include "module_init.h"

/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */
extern int 		agentHttpAccess;	 	// total accesos HTTP del agente
extern int 		serverStatus;	// por defecto va apagado 
extern int 		last_status;
char serverRestart_str[MY_MAX_LEN];
time_t tiempo_inicial;

/*
 * our initialization routine, automatically called by the agent 
 * (to get called, the function name must match init_FILENAME()) 
 */
void
init_serverRestart(void)
{
   static oid      serverRestart_oid[] = { MOD_SNMP_OID 1 , 4};
   
   DEBUGMSGTL(("serverRestart","Initializing the serverRestart module\n"));

    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "serverRestart", 
            handle_serverRestart,
            serverRestart_oid,
            OID_LENGTH(serverRestart_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &serverRestart_str, sizeof(serverRestart_str),
            ASN_OCTET_STR, WATCHER_MAX_SIZE)
        );
        
    DEBUGMSGTL(("serverRestart",
                "Done initalizing serverRestart module\n"));
}


int
handle_serverRestart(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    
    switch (reqinfo->mode) {

    case MODE_GET:
        break;
    case MODE_SET_RESERVE1:
        tiempo_inicial = time(NULL);
	break;
    case MODE_SET_RESERVE2:
	break;
    case MODE_SET_FREE:
	break;
    case MODE_SET_ACTION:
	break;
    case MODE_SET_COMMIT:
        // Ok --> send restart notification
        agentHttpAccess = 0; 					// Initialize agent HTTP access
        send_restart_notification();				// send notification
        break;
    case MODE_SET_UNDO:
        break;

    default:
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

void send_restart_notification()
{
    /* For Trap sending */
    oid             objid_snmptrap[] = { NOTIFICATION_OID };
    size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

    /* NOTIFICATION-TYPE */
    oid             serverRestartNotification_oid[] = { MOD_SNMP_OID 4, 2 };
    size_t          serverRestartNotification_oid_len = OID_LENGTH(serverRestartNotification_oid);
    
    netsnmp_variable_list *serverRestartNotification_vars = NULL;
    
    DEBUGMSGTL(("serverRestart", "NOTIFICATION - Inicio\n"));
  	    
	    DEBUGMSGTL(("serverRestart", "NOTIFICATION - definicion de OID's a enviar 1\n"));
    
	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              objid_snmptrap, objid_snmptrap_len,
	                              ASN_OBJECT_ID,
	                              (u_char *) serverRestartNotification_oid,
	                              serverRestartNotification_oid_len * sizeof(oid));
	    
	    DEBUGMSGTL(("serverRestart", "NOTIFICATION - definicion de OID's a enviar 2\n"));
	    
	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              serverRestartNotification_oid, serverRestartNotification_oid_len,
	                              ASN_OCTET_STR,
	                              (u_char *) serverRestart_str,
	                              strlen(serverRestart_str));

          DEBUGMSGTL(("serverRestart", "NOTIFICATION - sending the trap\n"));
    	    send_v2trap(serverRestartNotification_vars);
	    DEBUGMSGTL(("serverRestart", "NOTIFICATION - cleaning up\n"));
    	    snmp_free_varbind(serverRestartNotification_vars);
}