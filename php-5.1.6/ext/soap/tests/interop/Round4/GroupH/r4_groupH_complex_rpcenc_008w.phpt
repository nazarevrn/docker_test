--TEST--
SOAP Interop Round4 GroupH Complex RPC Enc 008 (php/wsdl): echoMultipleFaults2(2)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
class BaseStruct {
    function BaseStruct($f, $s) {
        $this->floatMessage = $f;
        $this->shortMessage = $s;
    }
}
class ExtendedStruct extends BaseStruct {
    function ExtendedStruct($f, $s, $x1, $x2, $x3) {
        $this->BaseStruct($f,$s);
        $this->stringMessage = $x1;
        $this->intMessage = $x2;
        $this->anotherIntMessage = $x3;
    }
}
class MoreExtendedStruct extends ExtendedStruct {
    function MoreExtendedStruct($f, $s, $x1, $x2, $x3, $b) {
        $this->ExtendedStruct($f, $s, $x1, $x2, $x3);
        $this->booleanMessage = $b;
    }
}
$s1 = new BaseStruct(12.345,1);
$s2 = new ExtendedStruct(12.345,2,"arg",-3,5);
$s3 = new MoreExtendedStruct(12.345,3,"arg",-3,5,true);
$client = new SoapClient(dirname(__FILE__)."/round4_groupH_complex_rpcenc.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echoMultipleFaults2(2,$s1,$s2,$s3);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round4_groupH_complex_rpcenc.inc");
echo "ok\n";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://soapinterop.org/wsdl" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns2="http://soapinterop.org/types" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:echoMultipleFaults2><whichFault xsi:type="xsd:int">2</whichFault><param1 xsi:type="ns2:BaseStruct"><floatMessage xsi:type="xsd:float">12.345</floatMessage><shortMessage xsi:type="xsd:short">1</shortMessage></param1><param2 xsi:type="ns2:ExtendedStruct"><floatMessage xsi:type="xsd:float">12.345</floatMessage><shortMessage xsi:type="xsd:short">2</shortMessage><stringMessage xsi:type="xsd:string">arg</stringMessage><intMessage xsi:type="xsd:int">-3</intMessage><anotherIntMessage xsi:type="xsd:int">5</anotherIntMessage></param2><param3 xsi:type="ns2:MoreExtendedStruct"><floatMessage xsi:type="xsd:float">12.345</floatMessage><shortMessage xsi:type="xsd:short">3</shortMessage><stringMessage xsi:type="xsd:string">arg</stringMessage><intMessage xsi:type="xsd:int">-3</intMessage><anotherIntMessage xsi:type="xsd:int">5</anotherIntMessage><booleanMessage xsi:type="xsd:boolean">true</booleanMessage></param3></ns1:echoMultipleFaults2></SOAP-ENV:Body></SOAP-ENV:Envelope>
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:ns1="http://soapinterop.org/types" xmlns:ns2="http://soapinterop.org/wsdl" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>SOAP-ENV:Server</faultcode><faultstring>Fault in response to 'echoMultipleFaults2'.</faultstring><detail><ns2:part3 xsi:type="ns1:ExtendedStruct"><floatMessage xsi:type="xsd:float">12.345</floatMessage><shortMessage xsi:type="xsd:short">2</shortMessage><stringMessage xsi:type="xsd:string">arg</stringMessage><intMessage xsi:type="xsd:int">-3</intMessage><anotherIntMessage xsi:type="xsd:int">5</anotherIntMessage></ns2:part3></detail></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>
ok
