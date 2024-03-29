--TEST--
Bug #37278 (SOAP not respecting uri in __soapCall)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
$options = array(
  "location" => "test://",
  "uri"      => "http://bricolage.sourceforge.net/Bric/SOAP/Auth",
  "trace"    => 1);

$client = new SoapClient(null, $options);

$newNS = "http://bricolage.sourceforge.net/Bric/SOAP/Story";

try {
  $client->__soapCall("list_ids", array(), array("uri" => $newNS));
} catch (Exception $e) {
  print $client->__getLastRequest();
}
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://bricolage.sourceforge.net/Bric/SOAP/Story" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:list_ids/></SOAP-ENV:Body></SOAP-ENV:Envelope>