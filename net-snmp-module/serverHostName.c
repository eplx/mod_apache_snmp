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
#include "serverHostName.h"
#include "module_init.h"


/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */

char serverHostName_str[MY_MAX_LEN];
/*
 * our initialization routine, automatically called by the agent 
 * (to get called, the function name must match init_FILENAME()) 
 */
void
init_serverHostName(void)
{
   static oid      serverHostName_oid[] =
        { MOD_SNMP_OID 1 , 9};

    /*
     * a debugging statement.  Run the agent with -DserverHostName to see
     * the output of this debugging statement. 
     */
    DEBUGMSGTL(("serverHostName", "Initializing the serverHostName module\n"));

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

    netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "serverHostName", 
            NULL,
            serverHostName_oid,
            OID_LENGTH(serverHostName_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &serverHostName_str, sizeof(serverHostName_str),
            ASN_OCTET_STR, WATCHER_MAX_SIZE)
        );

    DEBUGMSGTL(("serverHostName", "Done initalizing serverHostName module\n"));
}
