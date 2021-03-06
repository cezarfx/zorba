import module namespace cdml = "http://zorba.io/modules/store/static/collections/dml";

import schema namespace kml = "http://earth.google.com/kml/2.1" at "kml21.xsd";

import module namespace kml-data = "http://www.mykml/data" at "sc1_ex1.xqlib";

for $placemark in cdml:collection(xs:QName("kml-data:placemarks"))
where contains($placemark/kml:description, "Wild Pigs")
return 
  insert node
    <kml:visibility>false</kml:visibility>
  after
    $placemark/kml:name;
