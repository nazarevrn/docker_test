--TEST--
SOAP Interop Round3 GroupD Import3 002 (php/wsdl): echoStructArray
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
$client = new SoapClient(dirname(__FILE__)."/round3_groupD_import3.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoStructArray(array($struct,$struct));
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round3_groupD_import3.inc");
echo "ok\n";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop/" xmlns:ns2="http://soapinterop.org/xsd" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns3="http://soapinterop.org/xsd2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoStructArray><inputArray SOAP-ENC:arrayType="ns2:SOAPStruct[2]" xsi:type="ns3:ArrayOfSOAPStruct"><item xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></item><item xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></item></inputArray></ns1:echoStructArray></SOAP-ENV:Body></SOAP-ENV:Envelope>
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop/" xmlns:ns2="http://soapinterop.org/xsd" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns3="http://soapinterop.org/xsd2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoStructArrayResponse><Result SOAP-ENC:arrayType="ns2:SOAPStruct[2]" xsi:type="ns3:ArrayOfSOAPStruct"><item xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></item><item xsi:type="ns2:SOAPStruct"><varString xsi:type="xsd:string">arg</varString><varInt xsi:type="xsd:int">34</varInt><varFloat xsi:type="xsd:float">325.325</varFloat></item></Result></ns1:echoStructArrayResponse></SOAP-ENV:Body></SOAP-ENV:Envelope>
ok
