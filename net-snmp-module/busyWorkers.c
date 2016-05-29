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

5/27/2016 - changed from ASN_GAUGE32 to ASN_INTEGER
*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/watcher.h>
#include "busyWorkers.h"
#include "module_init.h"

/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */

static unsigned long busyWorkers = 0;        /* XXX: set default value */
extern char serverTmpDir_str[MY_MAX_LEN];	 // interchange file

/*
 * our initialization routine, automatically called by the agent 
 * (to get called, the function name must match init_FILENAME()) 
 */
void
init_busyWorkers(void)
{
    static oid      busyWorkers_oid[] =
        {MOD_SNMP_OID 2, 3 };

    /*
     * a debugging statement.  Run the agent with -DbusyWorkers to see
     * the output of this debugging statement. 
     */
    DEBUGMSGTL(("busyWorkers", "Initializing the busyWorkers module\n"));


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
    DEBUGMSGTL(("busyWorkers",
                "Initializing busyWorkers scalar integer.  Default value = %d\n",
                busyWorkers));

    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "busyWorkers", 
            handler_busyWorkers,
            busyWorkers_oid,
            OID_LENGTH(busyWorkers_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &busyWorkers, sizeof(busyWorkers),
            ASN_INTEGER, WATCHER_MAX_SIZE)
        );


    DEBUGMSGTL(("busyWorkers", "Done initalizing busyWorkers module\n"));
}


int
handler_busyWorkers(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    switch (reqinfo->mode) {

    case MODE_GET:
	// 22/07/04 - EP - don´t need call check_estado here
	//	if (check_estado())		// si Apache está ejecutándose..
	//	{
	//		int x;
	//		DEBUGMSGTL(("busyWorkers", "send_http_get -->\n"));
	if (send_http_get("/ap2_snmp"))
	{
		FILE *fp;	// archivo de intercambio
	//    	DEBUGMSGTL(("busyWorkers","send_http_get.  valor= %d\n",x));
    		 if ((fp = fopen(serverTmpDir_str,"r")) == NULL)
			{
				DEBUGMSGTL(("busyWorkers", "cannot open temporal file."));
			}
		 else
		 	{
				DEBUGMSGTL(("busyWorkers", "Reading data from temporal file."));
				rewind(fp);
			 	fseek(fp,2*sizeof(unsigned long),SEEK_SET);
			 	fread(&busyWorkers,sizeof(unsigned long),1,fp);
				fclose(fp);
							
		    		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
			                        	(u_char *)
			                        	&busyWorkers
			                        	,
			                        	sizeof(busyWorkers)
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

