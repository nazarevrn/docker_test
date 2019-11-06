--TEST--
date_sunrise() and date_sunset() functions
--INI--
precision=13
--FILE--
<?php

putenv ("TZ=Asia/Jerusalem");

for($a=1;$a<=12;$a++){
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_TIMESTAMP,31.76670,35.23330,90.83,2)." ";
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_STRING,31.76670,35.23330,90.83,2)." ";
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_DOUBLE,31.76670,35.23330,90.83,2)."\n";
	
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_TIMESTAMP,31.76670,35.23330,90.83,2)." ";
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_STRING,31.76670,35.23330,90.83,2)." ";
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_DOUBLE,31.76670,35.23330,90.83,2)."\n";
}
?>
--EXPECT--
1041395864 06:37 6.629013145891
1041432452 16:47 16.79245111439
1044073855 06:30 6.515408927982
1044112463 17:14 17.23987028904
1046491495 06:04 6.082214503336
1046533075 17:37 17.63201103534
1049167581 05:26 5.439443811173
1049212774 17:59 17.99303572948
1051757532 04:52 4.870193412616
1051806007 18:20 18.33539050867
1054434776 04:32 4.548982718277
1054485647 18:40 18.67981294906
1057026949 04:35 4.597195637274
1057078197 18:49 18.83256339675
1059706409 04:53 4.891657508917
1059755837 18:37 18.62144070428
1062385999 05:13 5.222095112101
1062432291 18:04 18.08095716848
1064979098 05:31 5.527319921542
1065021952 17:25 17.43133913592
1067658845 05:54 5.901629287095
1067698274 16:51 16.85390245352
1070252387 06:19 6.329924268936
1070289382 16:36 16.60631260094