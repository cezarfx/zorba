import module namespace dml = "http://www.zorba-xquery.com/modules/store/static/collections/dml";

import module namespace ns = "http://www.example.com/example" at "../collection_001.xqdata";

dml:delete-nodes-first(xs:QName("ns:collection"), xs:unsignedLong(1));

