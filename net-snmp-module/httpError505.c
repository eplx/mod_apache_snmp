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
#include "httpError505.h"
#include "module_init.h"

/** Initializes the httpError505 module */
static int httpError505 = 0;

void
init_httpError505(void)
{
    static oid      httpError505_oid[] =
        { MOD_SNMP_OID 5, 505 };

    DEBUGMSGTL(("httpError505", "Initializing\n"));

    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("httpError505", handle_httpError505,
                             httpError505_oid,
                             OID_LENGTH(httpError505_oid),
                             HANDLER_CAN_RWRITE));
}

int
handle_httpError505(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    switch (reqinfo->mode) {

    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                 (u_char *)
                                 &httpError505                                
                                 ,
                                 sizeof(httpError505)
                                 );
        break;

    case MODE_SET_RESERVE1:
        //if (requests->requestvb->type != ASN_COUNTER ) {
		   httpError505++;                                   
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
