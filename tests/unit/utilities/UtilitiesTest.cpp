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
#include "utilities/HandledList.hpp"
#include "utilities/GUID.hpp"
#include "utilities/Noise.hpp"
#include "system/System.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define TEST_NO_MAIN
#include "acutest/acutest.h"

int life_notifier_comp(const int* A, const int* B) {
   return *A - *B;
}

void array_test(void) {
   ZoneScoped;
   {
      ctDynamicArray<int> d_arr = {};
      ctStaticArray<int, 64> s_arr = {};
      /*Reserve*/
      d_arr.Reserve(64);
      TEST_CHECK(d_arr.Capacity() >= 64);
      /*Resize*/
      d_arr.Resize(1);
      s_arr.Resize(1);
      TEST_CHECK(d_arr.Count() == 1);
      TEST_CHECK(s_arr.Count() == 1);
      /*Memset*/
      d_arr.Memset(0);
      TEST_CHECK(s_arr.Count() == 1);
      d_arr.Clear();
      /*Append*/
      for (int i = 0; i < 32; i++) {
         d_arr.Append(32 - i);
         s_arr.Append(32 - i);
         TEST_CHECK(d_arr.Last() == 32 - i);
         TEST_CHECK(s_arr.Last() == 32 - i);
      }
      /*Insert*/
      d_arr.Insert(-32, 0);
      s_arr.Insert(-32, 0);
      TEST_CHECK(d_arr[0] == -32);
      TEST_CHECK(s_arr[0] == -32);
      d_arr.Insert(-64, 8);
      s_arr.Insert(-64, 8);
      TEST_CHECK(d_arr[8] == -64);
      TEST_CHECK(s_arr[8] == -64);
      d_arr.Insert(-128, -2);
      s_arr.Insert(-128, -2);
      TEST_CHECK(d_arr[d_arr.Count() - 2] == -128);
      TEST_CHECK(s_arr[s_arr.Count() - 2] == -128);
      /*Remove*/
      size_t d_originalCount = d_arr.Count();
      size_t s_originalCount = s_arr.Count();
      TEST_CHECK(d_arr[d_arr.Count() - 2] == s_arr[s_arr.Count() - 2]);
      TEST_CHECK(d_arr[8] == s_arr[8]);
      TEST_CHECK(d_arr[0] == s_arr[0]);
      int origAtM2 = d_arr[d_arr.Count() - 2];
      int origAt8 = d_arr[8];
      int origAt0 = d_arr[0];
      d_arr.RemoveAt(-2);
      s_arr.RemoveAt(-2);
      d_arr.RemoveAt(8);
      s_arr.RemoveAt(8);
      d_arr.RemoveAt(0);
      s_arr.RemoveAt(0);
      TEST_CHECK(d_arr.Count() == d_originalCount - 3 && !d_arr.Exists(origAt0) &&
                 !d_arr.Exists(origAt8) && !d_arr.Exists(origAtM2));
      TEST_CHECK(s_arr.Count() == s_originalCount - 3 && !s_arr.Exists(origAt0) &&
                 !s_arr.Exists(origAt8) && !s_arr.Exists(origAtM2));
      /*Exists*/
      TEST_CHECK(d_arr.Exists(4));
      TEST_CHECK(s_arr.Exists(4));
      /*Find*/
      int* ln = NULL;
      ln = d_arr.FindPtr(12, 0, 1);
      TEST_ASSERT(ln != NULL);
      ln = d_arr.FindPtr(7, -1, -1);
      TEST_ASSERT(ln != NULL);
      ln = s_arr.FindPtr(12, 0, 1);
      TEST_ASSERT(ln != NULL);
      ln = s_arr.FindPtr(7, -1, -1);
      TEST_ASSERT(ln != NULL);
      /*Sort*/
      d_arr.QSort(life_notifier_comp);
      int last = -100;
      for (size_t i = 0; i < d_arr.Count(); i++) {
         TEST_CHECK(last <= d_arr[i]);
         last = d_arr[i];
      }
      /*Verify*/
      int previous = d_arr[0];
      for (int i = 1; i < d_arr.Count(); i++) {
         TEST_CHECK(!(d_arr[i] - 1 != previous));
         previous = d_arr[i];
      }
      /*End*/
      d_arr.Clear();
      s_arr.Clear();
      TEST_CHECK(d_arr.Count() == 0);
      TEST_CHECK(s_arr.Count() == 0);
   }
}

void dynamic_string_test(void) {
   ZoneScoped;
   ctStringUtf8 mystring = ctStringUtf8("Hello world!");
   TEST_CHECK(mystring == "Hello world!");
   TEST_CHECK(ctCStrEql(mystring.CStr(), "Hello world!"));
   mystring += " This is a test!";
   mystring += '?';
   mystring += ' ';
   TEST_CHECK(mystring == "Hello world! This is a test!? ");
   ctStringUtf8 str2 = ctStringUtf8("Very nice! ");
   mystring += str2;
   TEST_CHECK(mystring == "Hello world! This is a test!? Very nice! ");
   mystring.Printf(32, "The lucky number is: %u", 0);
   TEST_CHECK(mystring ==
              "Hello world! This is a test!? Very nice! The lucky number is: 0");
   ctStringUtf8 secondstring = mystring;
   mystring.ToUpper();
   TEST_CHECK(mystring ==
              "HELLO WORLD! THIS IS A TEST!? VERY NICE! THE LUCKY NUMBER IS: 0");
   mystring += "!!!";
   TEST_CHECK(mystring ==
              "HELLO WORLD! THIS IS A TEST!? VERY NICE! THE LUCKY NUMBER IS: 0!!!");
}

void file_path_test(void) {
   ZoneScoped;
   ctStringUtf8 path = "C:\\test\\bin\\cfg.ini";
   TEST_CHECK(path.FilePathGetName() == "cfg");
   TEST_CHECK(path.FilePathRemoveExtension() == "C:\\test\\bin\\cfg");
   TEST_CHECK(path.FilePathPop() == "C:\\test\\bin");
   TEST_CHECK(path.FilePathUnify() == "C:/test/bin");
   TEST_CHECK(path.FilePathAppend("/test.cfg") == "C:/test/bin/test.cfg");
}

void bloom_filter_test(void) {
   ZoneScoped;
   const int numTests = 500;
   ctBloomFilter<int32_t, 2048, 10> bloom;
   for (int32_t i = 0; i < numTests; i++) {
      bloom.Insert(i);
   }
   for (int32_t i = 0; i < numTests; i++) {
      TEST_CHECK(bloom.MightExist(i));
   }
   int32_t falsePositives = 0;
   for (int32_t i = numTests; i < numTests + numTests; i++) {
      if (bloom.MightExist(i)) { falsePositives++; }
   }
   /*ctDebugLog("Bloom filter test:"
              "\n\tCorrectly Found: %d"
              "\n\tFalse Negatives: %d"
              "\n\tFalse Positives: %d",
              numTests,
              0,
              falsePositives);*/
}

/* todo: port rest to new test framework */
void spacial_query_test(void) {
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
}

void hash_table_test(void) {
   ZoneScoped;
   {
      ctHashTable<int, uint32_t> hashTable;
      uint32_t findhash = 0;
      for (int i = 1; i < 500000; i++) {
         uint32_t hash = XXH32(&i, sizeof(int), 0);
         if (i == 156) { findhash = hash; }
         hashTable.Insert(hash, i);
      }
      int* iptr = hashTable.FindPtr(findhash);
      TEST_ASSERT(iptr != NULL);
   }
   {
      ctHashTable<char, uint32_t> hashTable;
      hashTable.Insert(1, 'A');
      hashTable.Insert(2, 'B');
      hashTable.Insert(3, 'C');
      hashTable.Insert(4, 'D');
      hashTable.Insert(5, 'E');
      hashTable.Insert(6, 'F');
      hashTable.Insert(7, 'G');
      TEST_CHECK(*hashTable.FindPtr(3) == 'C');
      hashTable.Remove(4);
      TEST_CHECK(!hashTable.Exists(4));

      for (auto itt = hashTable.GetIterator(); itt; itt++) {
         // ctDebugLog("Key: %d - Value: %c", itt.Key(), itt.Value());
      }
   }
   /*ctDebugLog("Hash Table (Worst Case Dynamic String)...");
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
   }*/
}

void json_test(void) {
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
}

void noise_test(void) {
   //uint8_t* image = new uint8_t[1024 * 1024 * 3];
   //for (int x = 0; x < 1024; x++) {
   //   for (int y = 0; y < 1024; y++) {
   //      float value = ctNoiseWhite3DScalar(ctVec3((float)x / 50, (float)y / 50, 0));
   //      ctVec3 color = ctVec3(value * 0.5f + 0.5f);

   //      /* curl */
   //      /*color =
   //        ctNoiseCurl(ctVec3((float)x / 50,
   //                           (float)y / 50,
   //                           0)) *
   //          0.5f +
   //        ctVec3(0.5f);  */
   //      
   //      //color = ctNoiseVorronoiRich(ctVec3((float)x / 50, (float)y / 50, 0));

   //      color = ctVec3(ctClamp(color.x, 0.0f, 1.0f),
   //                     ctClamp(color.y, 0.0f, 1.0f),
   //                     ctClamp(color.z, 0.0f, 1.0f));
   //      image[(x + y * 1024) * 3 + 0] = (uint8_t)(color.x * 255);
   //      image[(x + y * 1024) * 3 + 1] = (uint8_t)(color.y * 255);
   //      image[(x + y * 1024) * 3 + 2] = (uint8_t)(color.z * 255);
   //   }
   //}
   //const char* path = "NoiseTest.bmp";
   //stbi_write_bmp(path, 1024, 1024, 3, image);
   //ctSystemShowFileToDeveloper(path);
}

void math_3d_test(void) {
}

void handled_list_test(void) {
   ctDynamicArray<ctHandle> handles;
   ctDynamicArray<int*> pointers;
   ctHandledList<int> list;
   const int count = 128;
   for (int i = 0; i < count; i++) {
      handles.Append(list.Insert(i));
      pointers.Append(&list[handles.Last()]);
   }
   for (int i = 0; i < count; i++) {
      ctDebugLog("Handle: %d "
                 "Pointer: %p "
                 "Value By Pointer: %d "
                 "Value By Handle: %d",
                 handles[i],
                 pointers[i],
                 *pointers[i],
                 list[handles[i]]);
   }
}