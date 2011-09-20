import module namespace ddl = "http://www.zorba-xquery.com/modules/store/static/collections/ddl";
import module namespace dml = "http://www.zorba-xquery.com/modules/store/static/collections/dml";
import module namespace ns = "http://example.org/datamodule/" at "collections.xqdata";

declare namespace ann = "http://www.zorba-xquery.com/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %ann:sequential function local:ddl() {
  ddl:create($ns:collection_1);
  dml:insert-nodes($ns:collection_1, (<x/>,<y/>));
};

declare %ann:sequential function local:testa() {
  try {
    {
      dml:delete-node-first($ns:collection_1);
      exit returning dml:collection($ns:collection_1);
    }
  } catch * {
    exit returning ("a",$err:code, $err:description);
  }
};

declare %ann:sequential function local:testb() {
  try {
    {
      dml:delete-node-last($ns:collection_1);
      exit returning dml:collection($ns:collection_1);
    }
  } catch * {
    exit returning ("b",$err:code, $err:description);
  }
};

declare %ann:sequential function local:main() {
  local:ddl();
  (
    local:testa(),
    local:testb()
  )
};

local:main()
