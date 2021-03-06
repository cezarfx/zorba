<?
/*
 * Copyright 2006-2012 The FLWOR Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

require 'zorba_api_wrapper.php';

function test(Zorba $aZorba) {
  $filename = 'books.xml';
  $f = fopen($filename, 'r');
  $contents = fread($f, filesize($filename));
  fclose($f);

  $dataManager = $aZorba->getXmlDataManager();
  $docIter = $dataManager->parseXML($contents);
  $docIter->open();

  $doc = Item::createEmptyItem();
  $docIter->next($doc);

  $docIter->close();
  $docIter->destroy();

  $xquery = $aZorba->compileQuery(".");
  $dynCtx = $xquery->getDynamicContext();
  $dynCtx->setContextItem($doc);
  print $xquery->execute()."\n";
  
  return;
}


$store = InMemoryStore::getInstance();
$zorba = Zorba::getInstance($store);

print "Running: XQuery execute - Get Iterator and print info from its items\n";
test($zorba);
print "Success";

$zorba->shutdown();
InMemoryStore::shutdown($store);

?>