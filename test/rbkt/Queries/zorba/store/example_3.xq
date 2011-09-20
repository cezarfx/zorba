import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

import module namespace doc = "http://www.zorba-xquery.com/modules/store/dynamic/documents";

declare namespace xhtml = "http://www.w3.org/1999/xhtml";

let $mydoc := http:get-node("http://zorba-xquery.com/rest-tests/doc.html")[2]
return
  {
    doc:put("mydoc.xml", $mydoc); (: add the document with name mydoc.xml :)

    replace value of node
      doc:document("mydoc.xml")//xhtml:title
    with
      "Excel Functions";

    doc:document("mydoc.xml")//xhtml:title/text()
  }
