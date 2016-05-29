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
#include "agentHttpAddress.h"
#include "module_init.h"

/** Initializes the agentHttpAddress module */
char agentHttpAddress_str[MY_MAX_LEN];

void
init_agentHttpAddress(void)
{
    static oid      agentHttpAddress_oid[] =
        { MOD_SNMP_OID 1, 8 };

    DEBUGMSGTL(("agentHttpAddress", "Initializing\n"));
    
    
    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "agentHttpAddress", 
            NULL,
            agentHttpAddress_oid,
            OID_LENGTH(agentHttpAddress_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &agentHttpAddress_str, sizeof(agentHttpAddress_str),
            ASN_OCTET_STR, WATCHER_MAX_SIZE)
        );                             
    


}

