/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "utilities/Common.h"
#include "utilities/SpacialQuery.hpp"
#include "utilities/BloomFilter.hpp"
#include "utilities/SpacialQuery.hpp"

int life_notifier_comp(const int* A, const int* B) {
   return *A - *B;
}

int dynamic_array_test() {
   ZoneScoped;
   ctDebugLog("Dynamic Array");
   ctDebugWarning("Warning test");
   {
      ctDynamicArray<int> arr = {};
      /*Reserve*/
      ctDebugLog("Reserve");
      arr.Reserve(64);
      /*Memset*/
      ctDebugLog("Memset");
      arr.Memset(0);
      /*Append*/
      ctDebugLog("Append");
      for (int i = 0; i < 32; i++) {
         arr.Append(32 - i);
      }
      /*Insert*/
      ctDebugLog("Insert");
      arr.Insert(-32, 0);
      arr.Insert(-64, 8);
      arr.Insert(-128, -2);
      /*Remove*/
      ctDebugLog("Remove");
      arr.RemoveAt(-2);
      arr.RemoveAt(8);
      arr.RemoveAt(0);
      /*Exists*/
      ctDebugLog("Exists");
      if (arr.Exists(4)) { ctDebugLog("Found 4"); }
      /*Find*/
      ctDebugLog("Find");
      int* ln = NULL;
      ln = arr.FindPtr(12, 0, 1);
      ln = arr.FindPtr(7, -1, -1);
      /*Sort*/
      ctDebugLog("Sort");
      arr.QSort(life_notifier_comp);
      /*Hash*/
      ctDebugLog("Hash");
      /*Verify*/
      ctDebugLog("Verify");
      int previous = arr[0];
      for (int i = 1; i < arr.Count(); i++) {
         if (arr[i] - 1 != previous) { return -1; }
         previous = arr[i];
      }
      /*End*/
      arr.Clear();
      ctDebugLog("End");
   }
   return 0;
}

int static_array_test() {
   ZoneScoped;
   ctStaticArray<int, 32> arr;
   arr.SetBytes(0);
   for (int i = 0; i < 32; i++) {
      arr.Append(32 - i);
   }
   arr.RemoveLast();
   arr.Insert(8, 2);
   arr.Capacity();
   arr.QSort(0, arr.Count(), life_notifier_comp);
   arr.RemoveAt(0);
   return 0;
}

int dynamic_string_test() {
   ZoneScoped;
   ctStringUtf8 mystring = ctStringUtf8("Hello world!");
   if (mystring.Cmp("Hello world!") == 0) { ctDebugLog("Cmp 0"); }
   if (mystring == "Hello world!") { ctDebugLog("Compare"); };
   mystring += "This is a test!";
   mystring += '?';
   mystring += ' ';
   ctStringUtf8 str2 = ctStringUtf8("Very nice! ");
   mystring += str2;
   mystring.Printf(32, "The lucky number is: %u", ctXXHash32("LUCKY", 0));
   ctStringUtf8 secondstring = mystring;
   mystring.ToUpper();
   mystring += "!!!";
   ctDebugLog("My String is %s", mystring.CStr());
   ctDebugLog("My String is %s", secondstring.CStr());
   return 0;
}

int file_path_test() {
   ZoneScoped;
   ctDebugLog("File path...");
   ctStringUtf8 path = "C:\\test\\bin\\cfg.ini";
   ctDebugLog("Path is %s", path.CStr());
   ctDebugLog("Name is %s", path.FilePathGetName().CStr());
   ctDebugLog("No extension is %s", path.FilePathRemoveExtension().CStr());
   ctDebugLog("Pop is %s", path.FilePathPop().CStr());
   ctDebugLog("Append is %s", path.FilePathAppend("/test.cfg").CStr());
   ctDebugLog("Local is %s", path.FilePathLocalize().CStr());
   return 0;
}

int bloom_filter_test() {
   ZoneScoped;
   ctDebugLog("Bloom filter...");
   const int numTests = 500;
   ctBloomFilter<int32_t, 2048, 10> bloom;
   for (int32_t i = 0; i < numTests; i++) {
      bloom.Insert(i);
   }
   ctDebugLog("These must all exist!");
   for (int32_t i = 0; i < numTests; i++) {
      if (bloom.MightExist(i)) {
      } else {
         ctDebugError("A ENTRY WHICH EXISTS WAS NOT FOUND!!!");
         return -1;
      }
   }
   ctDebugLog("The less of these exist the better!");
   int32_t falsePositives = 0;
   for (int32_t i = numTests; i < numTests + numTests; i++) {
      if (bloom.MightExist(i)) { falsePositives++; }
   }
   ctDebugLog("Bloom filter test:"
              "\n\tCorrectly Found: %d"
              "\n\tFalse Negatives: %d"
              "\n\tFalse Positives: %d",
              numTests,
              0,
              falsePositives);
   return 0;
}

int spacial_query_test() {
   ZoneScoped;
   ctDebugLog("Spacial query...");
   ctDebugLog("Size of key %d", sizeof(ctSpacialCellKey));
   ctSpacialQuery spacial;
   spacial.Reserve(50000);
   ctHandle hndl = ctHandle();
   ctDebugLog("Insert...");
   for (int i = 0; i < 1000; i++) {
      spacial.Add(hndl, ctSpacialCellKey(ctVec3((float)i, 0, 0)));
   }
   spacial.Remove(hndl, ctSpacialCellKey(ctVec3(2, 0, 0)));
   ctDebugLog("Lookup...");
   for (int i = 0; i < 1000; i++) {
      spacial.GetBucketCount(ctSpacialCellKey(ctVec3((float)i, 0, 0)));
   }
   ctDebugLog("Finished!");
   return 0;
}

int hash_table_test() {
   ZoneScoped;
   ctDebugLog("Hash Table (POD)...");
   {
      ctHashTable<int, uint32_t> hashTable;
      uint32_t findhash = 0;
      for (int i = 1; i < 500000; i++) {
         uint32_t hash = XXH32(&i, sizeof(int), 0);
         if (i == 156) { findhash = hash; }
         hashTable.Insert(hash, i);
      }
      int* iptr = hashTable.FindPtr(findhash);
      if (iptr) { ctDebugLog("Number %d", *iptr); }
   }
   ctDebugLog("Hash Table (Iterate)...");
   {
      ctHashTable<char, uint32_t> hashTable;
      hashTable.Insert(1, 'A');
      hashTable.Insert(2, 'B');
      hashTable.Insert(3, 'C');
      hashTable.Insert(4, 'D');
      hashTable.Insert(5, 'E');
      hashTable.Insert(6, 'F');
      hashTable.Insert(7, 'G');

      for (auto itt = hashTable.GetIterator(); itt; itt++) {
         ctDebugLog("Key: %d - Value: %c", itt.Key(), itt.Value());
      }
   }
   ctDebugLog("Hash Table (Worst Case Dynamic String)...");
   {
       ctHashTable<ctStringUtf8, uint32_t> hashTable;
      uint32_t findhash = 0;
      for (int i = 1; i < 500000; i++) {
         ctStringUtf8 result;
         result.Printf(64, "Number %d", i);
         uint32_t hash = result.xxHash32();
         if (i == 156) { findhash = hash; }
         hashTable.Insert(hash, result);
      }
      ctStringUtf8* strptr = hashTable.FindPtr(findhash);
      if (strptr) { ctDebugLog("%s", strptr->CStr()); }
   }
   return 0;
}

int json_test() {
   ZoneScoped;
   ctJSONWriter jsonOut;
   ctStringUtf8 str = "";
   jsonOut.SetStringPtr(&str);

   /*{
     "firstName": "John",
     "lastName": "Smith",
     "isAlive": true,
     "age": 27,
     "address": {
       "streetAddress": "21 2nd Street",
       "city": "New York",
       "state": "NY",
       "postalCode": "10021-3100"
     },
     "phoneNumbers": [
       {
         "type": "home",
         "number": "212 555-1234"
       },
       {
         "type": "office",
         "number": "646 555-4567"
       }
     ],
     "children": [],
     "spouse": null
   }*/
   jsonOut.PushObject();
   jsonOut.DeclareVariable("TEST");
   jsonOut.PushObject();
   jsonOut.DeclareVariable("firstName");
   jsonOut.WriteString("John");
   jsonOut.DeclareVariable("lastName");
   jsonOut.WriteString("Smith");
   jsonOut.DeclareVariable("isAlive");
   jsonOut.WriteBool(true);
   jsonOut.DeclareVariable("age");
   jsonOut.WriteNumber(27);
   jsonOut.DeclareVariable("address");
   jsonOut.PushObject();
   jsonOut.DeclareVariable("streetAddress");
   jsonOut.WriteString("21 2nd Street");
   jsonOut.DeclareVariable("city");
   jsonOut.WriteString("New York");
   jsonOut.DeclareVariable("state");
   jsonOut.WriteString("NY");
   jsonOut.DeclareVariable("postalCode");
   jsonOut.WriteString("10021-3100");
   jsonOut.PopObject();
   jsonOut.DeclareVariable("phoneNumbers");
   jsonOut.PushArray();
   jsonOut.PushObject();
   jsonOut.DeclareVariable("type");
   jsonOut.WriteString("home");
   jsonOut.DeclareVariable("number");
   jsonOut.WriteString("212 555-1234");
   jsonOut.PopObject();
   jsonOut.PushObject();
   jsonOut.DeclareVariable("type");
   jsonOut.WriteString("office");
   jsonOut.DeclareVariable("number");
   jsonOut.WriteString("646 555-4567");
   jsonOut.PopObject();
   jsonOut.PopArray();
   jsonOut.DeclareVariable("children");
   jsonOut.PushArray();
   jsonOut.PopArray();
   jsonOut.DeclareVariable("spouse");
   jsonOut.WriteNull();
   jsonOut.PopObject();

   jsonOut.DeclareVariable("Second Object");
   jsonOut.PushObject();
   jsonOut.DeclareVariable("vectors");
   jsonOut.PushArray();
   jsonOut.PushArray();
   jsonOut.WriteNumber(1.0f);
   jsonOut.WriteNumber(2.0f);
   jsonOut.WriteNumber(3.0f);
   jsonOut.PopArray();
   jsonOut.PushArray();
   jsonOut.WriteNumber(4.0f);
   jsonOut.WriteNumber(5.0f);
   jsonOut.WriteNumber(6.0f);
   jsonOut.PopArray();
   jsonOut.PopArray();
   jsonOut.PopObject();
   jsonOut.PopObject();

   ctDebugLog(str.CStr());
   ctDebugLog("------------------------------------------");

   /*Read*/
   ctJSONReader reader;
   reader.BuildJsonForPtr(str.CStr(), str.ByteLength());
   ctJSONReadEntry entry;
   reader.GetRootEntry(entry);
   entry.GetObjectEntry("Second Object", entry);
   entry.GetObjectEntry("vectors", entry);
   entry.GetArrayEntry(0, entry);
   entry.GetArrayEntry(1, entry);
   float value = -90.0f;
   entry.GetNumber(value);
   ctStringUtf8 str2;
   entry.GetString(str2);
   ctDebugLog("%f", value);
   return 0;
}

int math_3d_test() {
   return 0;
}

void debugCallback(int level, const char* format, va_list args) {
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH - 1, format, args);
   TracyMessage(tmp, strlen(tmp));
   vprintf(format, args);
   putchar('\n');
}

int main(int argc, char* argv[]) {
   ZoneScoped;
   _ctDebugLogSetCallback(debugCallback);
   dynamic_array_test();
   static_array_test();
   dynamic_string_test();
   hash_table_test();
   json_test();
   math_3d_test();
   file_path_test();
   bloom_filter_test();
   spacial_query_test();
   return 0;
}