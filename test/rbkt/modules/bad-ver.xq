(: This module has an illegal version specification :)

module namespace vm = "http://www.zorba-xquery.com/modules/bad-ver";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.4.";

declare variable $vm:value as xs:string := "version 1";

declare function vm:hello() as xs:string {
  "hello"
};

declare function vm:version() as xs:string {
  $vm:value
};
