--TEST--
testing @ and error_reporting - 4
--FILE--
<?php

error_reporting(E_ALL);

function foo() {
	echo $undef;
	error_reporting(E_ALL|E_STRICT);
}


foo(@$var);

var_dump(error_reporting());

echo "Done\n";
?>
--EXPECTF--
Notice: Undefined variable: undef in %s on line %d
int(4095)
Done
