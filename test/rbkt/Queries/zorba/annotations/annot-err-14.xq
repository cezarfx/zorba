declare namespace ann = "http://zorba.io/annotations";

declare %ann:assignable %ann:nonassignable variable $var := 3;

$var := 5;
$var