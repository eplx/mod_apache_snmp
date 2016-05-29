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
#include "httpError400.h"
#include "module_init.h"

/** Initializes the httpError400 module */
static int httpError400 = 0;

void
init_httpError400(void)
{
    static oid      httpError400_oid[] =
        { MOD_SNMP_OID 5, 400 };

    DEBUGMSGTL(("httpError400", "Initializing\n"));

    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("httpError400", handle_httpError400,
                             httpError400_oid,
                             OID_LENGTH(httpError400_oid),
                             HANDLER_CAN_RWRITE));
}

int
handle_httpError400(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /*
     * We are never called for a GETNEXT if it's registered as a
     * "instance", as it's "magically" handled for us.  
     */

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one. 
     */

    switch (reqinfo->mode) {

    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                 (u_char *)
                                 &httpError400                                
                                 ,
                                 sizeof(httpError400)
                                 );
        break;

    case MODE_SET_RESERVE1:
        //if (requests->requestvb->type != ASN_COUNTER ) {
	httpError400++;                                   
        //}
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
