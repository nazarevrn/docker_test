--TEST--
SOAP Interop Round3 GroupD RPC Encoded 003 (php/wsdl): echoStruct
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
class SOAPStruct {
    function SOAPStruct($s, $i, $f) {
        $this->varString = $s;
        $this->varInt = $i;
        $this->varFloat = $f;
    }
}
$struct = new SOAPStruct('arg',34,325.325);
$client = new SoapClient(dirname(__FILE__)."/round3_groupD_rpcenc.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoStruct($struct);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_rpcenc.inc");
echo "ok\n";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/WSDLInteropTestRpcEnc" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns2="http://soapinterop.org/xsd" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoStruct><param0 xsi:type="ns2:SOAPStruct"><varFloat xsi:type="xsd:float">325.325</varFloat><varInt xsi:type="xsd:int">34</varInt><varString xsi:type="xsd:string">arg</varString></param0></ns1:echoStruct></SOAP-ENV:Body></SOAP-ENV:Envelope>
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/WSDLInteropTestRpcEnc" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns2="http://soapinterop.org/xsd" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoStructResponse><return xsi:type="ns2:SOAPStruct"><varFloat xsi:type="xsd:float">325.325</varFloat><varInt xsi:type="xsd:int">34</varInt><varString xsi:type="xsd:string">arg</varString></return></ns1:echoStructResponse></SOAP-ENV:Body></SOAP-ENV:Envelope>
ok
