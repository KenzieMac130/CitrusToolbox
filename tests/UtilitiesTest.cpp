#include "utilities/Common.h"

#include <stdexcept>

int life_notifier_comp(const int* A, const int* B) {
   return *A - *B;
}

int dynamic_array_test() {
   ctDebugLog("Dynamic Array");
   ctDebugWarning("Warning test");
   {
      ctDynamicArray<int> arr = {};
      /*Reserve*/
      ctDebugLog("Reserve");
      arr.Reserve(64);
      /*Memset*/
      ctDebugLog("Memset");
      arr.SetBytes(0);
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
      arr.QSort(0, arr.Count(), life_notifier_comp);
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
   ctStringUtf8 mystring = ctStringUtf8(u8"Hello world!");
   if (mystring.Cmp("Hello world!") == 0) { ctDebugLog("Cmp 0"); }
   if (mystring == "Hello world!") { ctDebugLog("Compare"); };
   mystring += u8"My name Borat!";
   mystring += '?';
   mystring += L'❤';
   mystring += ' ';
   ctStringUtf8 str2 = ctStringUtf8("Very nice! ");
   mystring += str2;
   mystring.Printf(32, "The lucky number is: %u", ctxxHash32("LUCKY", 0));
   ctStringUtf8 secondstring = mystring;
   mystring.ToUpper();
   mystring += "!!!";
   ctDebugLog("My String is %s", mystring.CStr());
   ctDebugLog("My String is %s", secondstring.CStr());
   return 0;
}

int hash_table_test() {
   ctHashTable<ctStringUtf8> table;
   table.Reserve(32);
   const ctStringUtf8 str2 = ctStringUtf8("Very nice!");
   const uint32_t str2Hash = str2.xxHash32();
   table.Insert(XXH32("A", 1, 0), "A");
   table.Insert(XXH32("B", 1, 0), "B");
   table.Insert(XXH32("C", 1, 0), "C");
   table.Insert(str2Hash, str2);
   table.Insert(XXH32("D", 1, 0), "D");
   table.Insert(XXH32("E", 1, 0), "E");
   table.Insert(XXH32("F", 1, 0), "F");
   const ctStringUtf8* pstr = table.FindPtr(str2Hash);
   if (pstr) { ctDebugLog(pstr->CStr()); }
   table.Remove(str2Hash);
   if (!table.Exists(str2Hash)) { ctDebugLog("Delete Success!"); }
   return 0;
}

int json_test() {
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

   jsonOut.DeclareVariable("DOOT");
   jsonOut.PushObject();
   jsonOut.DeclareVariable("arara");
   jsonOut.PushArray();
   jsonOut.PushArray();
   jsonOut.WriteNumber(1.0f);
   jsonOut.WriteNumber(2.0f);
   jsonOut.WriteNumber(3.0f);
   jsonOut.PopArray();
   jsonOut.PushArray();
   jsonOut.WriteNumber(1.0f);
   jsonOut.WriteNumber(2.0f);
   jsonOut.WriteNumber(3.0f);
   jsonOut.PopArray();
   jsonOut.PopArray();
   jsonOut.PopObject();
   jsonOut.PopObject();

   ctDebugLog(str.CStr());

   /*Read*/
   ctJSONReader reader;
   reader.BuildJsonForPtr(str.CStr(), str.ByteLength());
   ctJSONReader::Entry entry;
   reader.GetRootEntry(entry);
   size_t sz = entry.GetRaw(NULL, 0);
   char* arr = (char*)ctMalloc(sz + 1, "arr");
   ctAssert(arr);
   memset(arr, 0, sz + 1);
   entry.GetRaw(arr, sz);
   ctDebugLog(arr)
   return 0;
}

int main(int argc, char* argv[]) {
   dynamic_array_test();
   static_array_test();
   dynamic_string_test();
   hash_table_test();
   json_test();
   return 0;
}