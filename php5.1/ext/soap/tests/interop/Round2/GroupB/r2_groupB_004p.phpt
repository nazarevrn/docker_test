--TEST--
SOAP Interop Round2 groupB 004 (php/direct): echoNestedStruct
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
$param = (object)array(
  'varString' => "arg",
  'varInt' => 34,
  'varFloat' => 123.45,
  'varStruct' => (object)array(
    'varString' => "arg2",
    'varInt' => 342,
    'varFloat' => 123.452,
  ));

$client = new SoapClient(NULL,array("location"=>"test://","uri"=>"http://soapinterop.org/","trace"=>1,"exceptions"=>0));
$client->__soapCall("echoNestedStruct", array($param), array("soapaction"=>"http://soapinterop.org/","uri"=>"http://soapinterop.org/"));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoNestedStruct><param0 xsi:type="SOAP-ENC:Struct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">123.45</varFloat><varStruct xsi:type="SOAP-ENC:Struct"><varString xsi:type="xsd:string">arg2</varString><varInt xsi:type="xsd:int">342</varInt><varFloat xsi:type="xsd:float">123.452</varFloat></varStruct></param0></ns1:echoNestedStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns2="http://soapinterop.org/xsd" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoNestedStructResponse><return xsi:type="ns2:SOAPStructStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">123.45</varFloat><varStruct xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg2</varString><varInt xsi:type="xsd:int">342</varInt><varFloat xsi:type="xsd:float">123.452</varFloat></varStruct></return></ns1:echoNestedStructResponse></SOAP-ENV:Body></SOAP-ENV:Envelope>
ok
