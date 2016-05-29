#!/bin/sh

# Paths - Modify if you need...
apachepath="/usr/local/apache2"
libs_net_snmp="/usr/local/lib"
incs_net_snmp="/usr/local/include/net-snmp"

# Don't change this..
$apachepath/bin/apxs -a -i -c -L$libs_net_snmp -I$incs_net_snmp  mod_ap2_snmp.c -lnetsnmp -lcrypto -ldl -O2

# For FreeBSD
#/usr/local/sbin/apxs -a -i -c -L$libs_net_snmp -I$incs_net_snmp  mod_ap2_snmp.c -lnetsnmp -lcrypto -O2

# Use it if you need debugging.
#$apachepath/bin/apxs -a -i -c -L$libs_net_snmp -I$incs_net_snmp  mod_ap2_snmp.c -lnetsnmp -lcrypto -ldl -g

