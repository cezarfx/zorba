let $message := fn:parse-xml("<?xml version='1.0'?><a><b>textnode</b></a>")
return fn:serialize($message)
