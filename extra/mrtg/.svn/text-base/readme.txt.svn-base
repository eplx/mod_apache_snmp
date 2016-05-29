MRTG configuration examples for mod-apache-snmp.
-----------------------------------------------

Objetive: 
Give some examples about obtaining mrtg graphics from Apache Web server (using mod-apache-snmp).

Limits:
There are only examples for the following MIB objects:
APACHE2-MIB::busyWorkers.0		// busy workers
APACHE2-MIB::idleWorkers.0		// idle workers
APACHE2-MIB::serverKBytesPerRequest.0	// Kbytes / Request
APACHE2-MIB::serverKBytesPerSec.0	// Kbytes / Second
APACHE2-MIB::serverRequestsPerSec.0	// Requests / Second

Requirements:
1) Apache Web Server 2.0.x - http://www.apache.org/
2) mod-apache-snmp 1.02 (or later) - http://mod-apache-snmp.sourceforge.net/
3) MRTG (tested with version 2.5.4c, but it should work with older versions too.. :) ) - http://www.mrtg.org/

Description:
The file mrtg.cfg.examples include mrtg configuration directives for those mib objects. 

Note, that some of them (the ones that return DisplayString values, for example: serverKBytesPerSec)
should use an auxiliar script for converting string to numbers.. 
This is in that way because mrtg has to read *number* values and not *strings*.


--------------------------------------------------------------------------------------
mod-apache-snmp project - http://mod-apache-snmp.sourceforge.net/
Author / Mantainer: Esteban Pizzini - e-mail: ep@fibertel.com.ar



