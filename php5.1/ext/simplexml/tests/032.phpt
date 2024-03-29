--TEST--
SimpleXML: comparing instances
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?>
--FILE--
<?php 
$xml =<<<EOF
<people>
  <person name="Joe"/>
  <person name="John">
    <children>
      <person name="Joe"/>
    </children>
  </person>
  <person name="Jane"/>
</people>
EOF;

$xml1 =<<<EOF
<people>
  <person name="John">
    <children>
      <person name="Joe"/>
    </children>
  </person>
  <person name="Jane"/>
</people>
EOF;


$people = simplexml_load_string($xml);
$people1 = simplexml_load_string($xml);
$people2 = simplexml_load_string($xml1);

var_dump($people1 == $people);
var_dump($people2 == $people);
var_dump($people2 == $people1);

?>
===DONE===
--EXPECTF--
bool(true)
bool(false)
bool(false)
===DONE===
