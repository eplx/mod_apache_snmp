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
#include "totalServerPorts.h"
#include "module_init.h"

/** Initializes the totalServerPorts module */
int totalServerPorts = 0;

void
init_totalServerPorts(void)
{
    static oid      totalServerPorts_oid[] =
        { MOD_SNMP_OID 1, 9 };

    DEBUGMSGTL(("totalServerPorts", "Initializing\n"));

    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "totalServerPorts", 
            handle_totalServerPorts,
            totalServerPorts_oid,
            OID_LENGTH(totalServerPorts_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &totalServerPorts, sizeof(totalServerPorts),
            ASN_INTEGER, WATCHER_MAX_SIZE)
        );                             
}

int
handle_totalServerPorts(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    switch (reqinfo->mode) {

    case MODE_GET:
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
	clean_table(); // Clean table
        break;

    case MODE_SET_UNDO:
        break;

    default:
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
