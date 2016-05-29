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
#include "serverKBytesPerSec.h"
#include "module_init.h"


/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */

static char serverKBytesPerSec_str[MY_MAX_LEN];       	/* Init serverKBytesPerSec */

extern char serverTmpDir_str[MY_MAX_LEN];	 	/* interchange file*/

/*
 * our initialization routine, automatically called by the agent
 * (to get called, the function name must match init_FILENAME())
 */
void
init_serverKBytesPerSec(void)
{
    static oid      serverKBytesPerSec_oid[] =
        { MOD_SNMP_OID 2, 9 };

    /*
     * a debugging statement.  Run the agent with -DserverKBytesPerSec to see
     * the output of this debugging statement.
     */
    DEBUGMSGTL(("serverKBytesPerSec", "Initializing the serverKBytesPerSec module\n"));

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
    DEBUGMSGTL(("serverKBytesPerSec",
                "Initializing serverKBytesPerSec scalar integer.  Default value = %s\n",
                serverKBytesPerSec_str));

     netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "serverKBytesPerSec",
            handler_serverKBytesPerSec,
            serverKBytesPerSec_oid,
            OID_LENGTH(serverKBytesPerSec_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &serverKBytesPerSec_str, sizeof(serverKBytesPerSec_str),
            ASN_OCTET_STR, WATCHER_MAX_SIZE)
        );

    DEBUGMSGTL(("serverKBytesPerSec", "Done initalizing serverKBytesPerSec module\n"));
}


int
handler_serverKBytesPerSec(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    // temp variable for reading tmp file.
    float serverKBytesPerSec_tmp = 0;

    switch (reqinfo->mode) {

    case MODE_GET:

	if (send_http_get("/ap2_snmp"))
	{
		FILE *fp;	// interchange filehandler
    		//DEBUGMSGTL(("serverKBytesPerSec","send_http_get.  valor= %d\n",x));

    		 if ((fp = fopen(serverTmpDir_str,"r")) == NULL)
			{
				DEBUGMSGTL(("serverKBytesPerSec", "cannot open temporal file."));
			}
		 else
		 	{
				DEBUGMSGTL(("serverKBytesPerSec", "Reading data from temporal file."));
				rewind(fp);
			 	fseek(fp,2*sizeof(unsigned long)+sizeof(int)*2+sizeof(float),SEEK_SET);
			 	fread(&serverKBytesPerSec_tmp,sizeof(float),1,fp);
				fclose(fp);

				sprintf(serverKBytesPerSec_str,"%f",serverKBytesPerSec_tmp);

				snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)
										serverKBytesPerSec_str
										,
										strlen(serverKBytesPerSec_str)-1
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
