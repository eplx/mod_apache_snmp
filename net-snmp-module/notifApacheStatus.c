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

/*
 * start be including the appropriate header files 
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * contains prototypes 
 */
#include "notifApacheStatus.h"

extern char serverRestart_str[MY_MAX_LEN];
/*
 * our initialization routine
 * (to get called, the function name must match init_FILENAME() 
 */
void
init_notifApacheStatus(void)
{
    DEBUGMSGTL(("notifApacheStatus",
                "initializing (setting callback alarm)\n"));
    snmp_alarm_register(30,     /* seconds */
                        SA_REPEAT,      /* repeat (every 30 seconds). */
                        send_example_notifApacheStatus,      /* our callback */
                        NULL    /* no callback data needed */
        );
}

/** here we send a SNMP v2 trap (which can be sent through snmpv3 and
 *  snmpv1 as well) and send it out.
 *
 *     The various "send_trap()" calls allow you to specify traps in different
 *  formats.  And the various "trapsink" directives allow you to specify
 *  destinations to receive different formats.
 *  But *all* traps are sent to *all* destinations, regardless of how they
 *  were specified.
 *  
 *  
 *  I.e. it's
 *                                           ___  trapsink
 *                                          /
 *      send_easy_trap \___  [  Trap      ] ____  trap2sink
 *                      ___  [ Generator  ]
 *      send_v2trap    /     [            ] ----- informsink
 *                                          \____
 *                                                trapsess
 *  
 *  *Not*
 *       send_easy_trap  ------------------->  trapsink
 *       send_v2trap     ------------------->  trap2sink
 *       ????            ------------------->  informsink
 *       ????            ------------------->  trapsess
 */
void
send_example_notifApacheStatus(unsigned int clientreg, void *clientarg)
{

    /*
     * In the notifApacheStatus, we have to assign our notifApacheStatus OID to
     * the snmpTrapOID.0 object. Here is it's definition. 
     */
    /* OID snmpTrapOID.0 object */
    oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

    oid             notifApacheStatus_oid[] = { 1, 3, 6, 1, 4, 1, 19786, 1, 1, 4, 1 };
    size_t          notifApacheStatus_oid_len = OID_LENGTH(notifApacheStatus_oid);
    
    oid             example_string_oid[] = {1, 3, 6, 1, 4, 1, 19786, 1, 1, 1 , 6 };
    
    /*
     * here is where we store the variables to be sent in the trap 
     */
    netsnmp_variable_list *notifApacheStatus_vars = NULL;

    DEBUGMSGTL(("notifApacheStatus", "defining the trap\n"));


    snmp_varlist_add_variable(&notifApacheStatus_vars,
                              objid_snmptrap, objid_snmptrap_len,
                              ASN_OBJECT_ID,
                              (u_char *) notifApacheStatus_oid,
                              notifApacheStatus_oid_len * sizeof(oid));

    /*
     * if we wanted to insert additional objects, we'd do it here 
     */
    snmp_varlist_add_variable(&notifApacheStatus_vars,
                              notifApacheStatus_oid, notifApacheStatus_oid_len,
                              ASN_OCTET_STR,
                              (u_char *) serverRestart_str,
                              strlen(serverRestart_str));


    /*
     * send the trap out.  This will send it to all registered
     * receivers (see the "SETTING UP TRAP AND/OR INFORM DESTINATIONS"
     * section of the snmpd.conf manual page. 
     */
    DEBUGMSGTL(("notifApacheStatus", "sending the trap\n"));
    send_v2trap(notifApacheStatus_vars);

    /*
     * free the created notifApacheStatus variable list 
     */
    DEBUGMSGTL(("notifApacheStatus", "cleaning up\n"));
    snmp_free_varbind(notifApacheStatus_vars);
}
