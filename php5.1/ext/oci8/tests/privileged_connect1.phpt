--TEST--
privileged connect tests
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--INI--
oci8.privileged_connect=1
--FILE--
<?php
		
require dirname(__FILE__)."/connect.inc";

oci_connect("", "", "", false, OCI_SYSOPER);
oci_connect("", "", "", false, OCI_SYSDBA);
oci_connect("", "", "", false, -1);
oci_connect("", "", "", false, "qwe");

echo "Done\n";
?>
--EXPECTF--	
Warning: oci_connect(): ORA-01031: insufficient privileges in %s on line %d

Warning: oci_connect(): ORA-01031: insufficient privileges in %s on line %d

Warning: oci_connect(): Invalid session mode specified (-1) in %s on line %d

Warning: oci_connect() expects parameter 5 to be long, string given in %s on line %d
Done
