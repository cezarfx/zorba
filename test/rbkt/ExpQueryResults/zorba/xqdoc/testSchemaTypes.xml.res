<?xml version="1.0" encoding="UTF-8"?>
<xqdoc:xqdoc xmlns:xqdoc="http://www.xqdoc.org/1.0"><xqdoc:control><xqdoc:date/><xqdoc:version>1.0</xqdoc:version></xqdoc:control><xqdoc:module type="library"><xqdoc:uri>http://www.example.com/testModule</xqdoc:uri><xqdoc:name>image.xqlib</xqdoc:name><xqdoc:custom tag="namespaces"><xqdoc:namespace prefix="image" uri="http://www.zorba-xquery.com/modules/image/image"/><xqdoc:namespace prefix="testModule" uri="http://www.example.com/testModule"/></xqdoc:custom></xqdoc:module><xqdoc:imports><xqdoc:import type="schema"><xqdoc:uri>http://www.zorba-xquery.com/modules/image/image</xqdoc:uri></xqdoc:import></xqdoc:imports><xqdoc:variables/><xqdoc:functions><xqdoc:function arity="1"><xqdoc:name>testModule:test</xqdoc:name><xqdoc:signature>declare function testModule:test($image as image:imageType)</xqdoc:signature></xqdoc:function><xqdoc:function arity="1"><xqdoc:name>testModule:test2</xqdoc:name><xqdoc:signature>declare function testModule:test2($image as xs:string)</xqdoc:signature><xqdoc:invoked arity="1"><xqdoc:uri>http://www.example.com/testModule</xqdoc:uri><xqdoc:name>test</xqdoc:name></xqdoc:invoked><xqdoc:invoked arity="1"><xqdoc:uri>http://www.zorba-xquery.com/modules/image/image</xqdoc:uri><xqdoc:name>imageType</xqdoc:name></xqdoc:invoked></xqdoc:function></xqdoc:functions></xqdoc:xqdoc>