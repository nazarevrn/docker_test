--TEST--
SOAP Interop Round4 GroupH Complex RPC Enc 006 (php/wsdl): echoMultipleFaults1(3)
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
class BaseStruct {
    function BaseStruct($f, $s) {
        $this->floatMessage = $f;
        $this->shortMessage = $s;
    }
}
$s1 = new SOAPStruct('arg',34,325.325);
$s2 = new BaseStruct(12.345,12);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_complex_rpcenc.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoMultipleFaults1(3,$s1,$s2);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_complex_rpcenc.inc");
echo "ok\n";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/wsdl" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns2="http://soapinterop.org/types" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoMultipleFaults1><whichFault xsi:type="xsd:int">3</whichFault><param1 xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></param1><param2 xsi:type="ns2:BaseStruct"><floatMessage xsi:type="xsd:float">12.345</floatMessage><shortMessage xsi:type="xsd:short">12</shortMessage></param2></ns1:echoMultipleFaults1></SOAP-ENV:Body></SOAP-ENV:Envelope>
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns1="http://soapinterop.org/types" xmlns:ns2="http://soapinterop.org/wsdl" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>SOAP-ENV:Server</faultcode><faultstring>Fault in response to 'echoMultipleFaults1'.</faultstring><detail><ns2:part1 xsi:type="ns1:SOAPStructFault"><soapStruct xsi:type="ns1:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></soapStruct></ns2:part1></detail></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>
ok
