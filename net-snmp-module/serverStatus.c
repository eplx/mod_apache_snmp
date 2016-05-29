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
#include "serverStatus.h"
#include "module_init.h"

extern char serverRestart_str[MY_MAX_LEN];
extern char serverName_str[MY_MAX_LEN];
/*extern char serverRoot_str[MY_MAX_LEN];*/
extern char serverPidfile_str[MY_MAX_LEN];

int serverStatus = 0;	// set default value (down)
int last_status = 0;	// last status value

void
init_serverStatus(void)
{
    static oid      serverStatus_oid[] =
        { MOD_SNMP_OID 2, 5 };

    DEBUGMSGTL(("serverStatus", "Register scalar\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("serverStatus", handle_serverStatus,
                             serverStatus_oid,
                             OID_LENGTH(serverStatus_oid),
                             HANDLER_CAN_RWRITE));

    DEBUGMSGTL(("serverStatus", "Register notification alarm\n"));
    snmp_alarm_register(30,     /* seconds */
                        SA_REPEAT,      /* repeat (every 30 seconds). */
                        send_status_notification,      /* our callback */
                        NULL    /* no callback data needed */
		        );

}

int
handle_serverStatus(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one.
     */

    switch (reqinfo->mode) {

    case MODE_GET:
	serverStatus = check_estado();
	snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 (u_char *)
                                 &serverStatus
                                 ,
                                 sizeof(serverStatus)
                                 );
        break;
    case MODE_SET_RESERVE1:
	serverStatus = *requests->requestvb->val.integer;
	send_notification(serverStatus);
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
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

void send_status_notification(unsigned int clientreg, void *clientarg)
{
    /* Para el envio de traps */
    oid             objid_snmptrap[] = { NOTIFICATION_OID };
    size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

    /* NOTIFICATION-TYPE */
    oid             serverRestartNotification_oid[] = { MOD_SNMP_OID 4, 1 };
    size_t          serverRestartNotification_oid_len = OID_LENGTH(serverRestartNotification_oid);

    netsnmp_variable_list *serverRestartNotification_vars = NULL;

    DEBUGMSGTL(("serverStatus", "NOTIFICATION - Init\n"));
    serverStatus = check_estado();
    DEBUGMSGTL(("serverStatus", "NOTIFICATION -%d %d\n",serverStatus,last_status));
    if (last_status != serverStatus)
    {
	    last_status = serverStatus;

	    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending 1\n"));

	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              objid_snmptrap, objid_snmptrap_len,
	                              ASN_OBJECT_ID,
	                              (u_char *) serverRestartNotification_oid,
	                              serverRestartNotification_oid_len * sizeof(oid));

	    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending  2\n"));
	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              serverRestartNotification_oid, serverRestartNotification_oid_len,
	                              ASN_OCTET_STR,
	                              (u_char *) serverName_str,
	                              strlen(serverName_str));

	    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending  3\n"));
	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              serverRestartNotification_oid, serverRestartNotification_oid_len,
	                              ASN_INTEGER,
	                              (u_char *) &serverStatus,
	                              sizeof(serverStatus));

            DEBUGMSGTL(("serverStatus", "NOTIFICATION - sending trap\n"));
    	    send_v2trap(serverRestartNotification_vars);
	    DEBUGMSGTL(("serverStatus", "NOTIFICATION - cleaning up\n"));
    	    snmp_free_varbind(serverRestartNotification_vars);
	}
}

void send_notification(int status)
{
    /* For trap sending */
    oid             objid_snmptrap[] = { NOTIFICATION_OID };
    size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

    /* NOTIFICATION-TYPE */
    oid             serverRestartNotification_oid[] = { MOD_SNMP_OID 4, status };
    size_t          serverRestartNotification_oid_len = OID_LENGTH(serverRestartNotification_oid);

    netsnmp_variable_list *serverRestartNotification_vars = NULL;

    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending  1\n"));

    serverStatus = status;

    snmp_varlist_add_variable(&serverRestartNotification_vars,
                              objid_snmptrap, objid_snmptrap_len,
                              ASN_OBJECT_ID,
                              (u_char *) serverRestartNotification_oid,
                              serverRestartNotification_oid_len * sizeof(oid));

    	    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending  2\n"));
	    snmp_varlist_add_variable(&serverRestartNotification_vars,
	                              serverRestartNotification_oid, serverRestartNotification_oid_len,
	                              ASN_OCTET_STR,
	                              (u_char *) serverName_str,
	                              strlen(serverName_str));

    DEBUGMSGTL(("serverStatus", "NOTIFICATION - OID for sending  3\n"));
    snmp_varlist_add_variable(&serverRestartNotification_vars,
                              serverRestartNotification_oid, serverRestartNotification_oid_len,
                              ASN_INTEGER,
                              (u_char *) &serverStatus,
                              sizeof(serverStatus));

    DEBUGMSGTL(("serverStatus", "NOTIFICATION - sending the trap\n"));
    send_v2trap(serverRestartNotification_vars);
    DEBUGMSGTL(("serverStatus", "NOTIFICATION - cleaning up\n"));
    snmp_free_varbind(serverRestartNotification_vars);
}

/* check apache status */
int check_estado()
{
	int	estado;
	FILE	*fp = NULL;

	DEBUGMSGTL(("serverStatus", "FCN - check_estado - Init\n"));

/*	if ((fp = fopen(serverPidfile_str,"r"))==NULL)
    		{
			DEBUGMSGTL(("serverStatus", "FCN - check_estado - Can't open Pidfile\n"));
		    DEBUGMSGTL(("serverStatus", "Pidfile -%s\n",serverPidfile_str));
			estado = SERVER_DOWN;	//Apagado
		}
	else
		{
			if (send_http_get("/ap2_snmp/status"))
			{
    				estado = SERVER_UP;		// it´s running and response ok
    				DEBUGMSGTL(("serverStatus", "FCN - check_estado - Ok\n"));

			}
			else
			{
				estado = SERVER_NOT_RESPONDING;	// Not responding
    				DEBUGMSGTL(("serverStatus", "FCN - check_estado - Not responding\n"));

			}

			fclose(fp);
		}

*/
			// First check if apache is responding
			if (send_http_get("/ap2_snmp/status") == 1)
			{
    				estado = SERVER_UP;		// it´s running and response ok
    				DEBUGMSGTL(("serverStatus", "FCN - check_estado - Ok\n"));

			}
			// if not, check that pidfile doesn't exist
			else if ((fp = fopen(serverPidfile_str,"r"))==NULL)
    		{
					DEBUGMSGTL(("serverStatus", "FCN - check_estado - Can't open Pidfile\n"));
			  	    DEBUGMSGTL(("serverStatus", "Pidfile -%s\n",serverPidfile_str));
					estado = SERVER_DOWN;	//Apagado
			}
			// if apache doesn't work and pidfile exist, apache is not responding
			else
			{
					estado = SERVER_NOT_RESPONDING;	// Not responding
    				DEBUGMSGTL(("serverStatus", "FCN - check_estado - Not responding\n"));

			}

			// if file is open -> we close
			if (fp != NULL) {
					DEBUGMSGTL(("serverStatus", "FCN - check_estado - Close file (PidFile)\n"));
					fclose(fp);
			}


	DEBUGMSGTL(("serverStatus", "FCN - check_estado - End (%d)\n",estado));
	return(estado);
}
