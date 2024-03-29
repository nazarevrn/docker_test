--TEST--
collections and negative/too big element indexes
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";

$ora_sql = "DROP TYPE
						".$type_name."
		   ";

$statement = OCIParse($c,$ora_sql);
@OCIExecute($statement);

$ora_sql = "CREATE TYPE ".$type_name." AS TABLE OF NUMBER";
			  
$statement = OCIParse($c,$ora_sql);
OCIExecute($statement);


$coll1 = ocinewcollection($c, $type_name);

var_dump(oci_collection_append($coll1, 1));
var_dump(oci_collection_element_assign($coll1,-1,2345));
var_dump(oci_collection_element_assign($coll1,5000,2345));
var_dump(oci_collection_element_get($coll1, -1));
var_dump(oci_collection_element_get($coll1, -100));
var_dump(oci_collection_element_get($coll1, 500));

echo "Done\n";

require dirname(__FILE__)."/drop_type.inc";

?>
--EXPECTF--
bool(true)

Warning: oci_collection_element_assign(): OCI-22165: given index [4294967295] must be in the range of 0 to [0] in %s on line %d
bool(false)

Warning: oci_collection_element_assign(): OCI-22165: given index [5000] must be in the range of 0 to [0] in %s on line %d
bool(false)
bool(false)
bool(false)
bool(false)
Done
