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
using System;
using System.Text;
using org.zorbaxquery.api;

namespace ZorbaApplication
{

    class MyDiagnosticHandler : DiagnosticHandler {
        public override void error(ZorbaException exception) {
          System.Console.WriteLine("Exception catched in MyDiagnosticHandler");
          System.Console.WriteLine("Description: " + exception.getDescription());
        }
    }

    class Program
    {
        static String test(String query) {
            MyDiagnosticHandler diagnosticHandler = new MyDiagnosticHandler();

            InMemoryStore store = InMemoryStore.getInstance();
            Zorba zorba = Zorba.getInstance(store);

            XQuery xquery = zorba.compileQuery(query, diagnosticHandler);
            
            StringBuilder sbuilder = new StringBuilder();
            sbuilder.Append(xquery.execute());

            xquery.destroy();
            xquery.Dispose();
            
            zorba.shutdown();
            InMemoryStore.shutdown(store);
            
            return sbuilder.ToString();
        }

        static void Main(string[] args)
        {
            System.Console.WriteLine("Running: Compile query string using JSONiq");

            String query = @" 
              let $sats:=jn:json-doc('@CSharpPath@/tests/satellites.json')('satellites') 
              return { 
               'visible' : [ 
                  for $sat in jn:keys($sats) 
                  where $sats($sat)('visible') 
                  return $sat 
               ], 
               'invisible' : [ 
                  for $sat in jn:keys($sats) 
                  where not($sats($sat)('visible')) 
                  return $sat 
               ] 
              } 
            ";
            
            String testResult = "{ \"visible\" : [ \"AJISAI (EGS)\", \"AKARI (ASTRO-F)\" ], \"invisible\" : [ \"AAU CUBESAT\" ] }";
            System.Console.WriteLine("Query: " + query);
            String result;
            try {
                result = test(query);
            } catch(Exception e) {
                System.Console.WriteLine("Failed");
                Console.WriteLine("{0} Exception caught.", e);
                return;
            }
            
            System.Console.WriteLine("Expecting: " + testResult);
            System.Console.WriteLine("Result: " + result);
            
            if (result.Equals(testResult)) {
                System.Console.WriteLine("Success");
            } else {
                System.Console.WriteLine("Failed");
            }

            
        }
    }
}
