declare namespace ann = "http://www.zorba-xquery.com/annotations";

declare %ann:assignable %ann:nonassignable variable $var := 3;

$var := 5;
$var