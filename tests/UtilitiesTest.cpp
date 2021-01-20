#include "utilities/Common.hpp"

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

int dynamic_string_test()
{
    ctStringUtf8 mystring = u8"UUUUUWUUUおはいおおお!";
    mystring += u8"おはいおおお!";
    mystring += '?';
    mystring += L'❤';
    mystring += ctStringUtf8("DOOT");
    mystring.Printf(32, "Number is: %d", 233969);
    ctStringUtf8 secondstring = mystring;
    mystring.ToUpper();
    mystring += "!!!";
    ctDebugLog("My String is %s", mystring.CStr());
    ctDebugLog("My String is %s", secondstring.CStr());
    return 0;
}

int main(int argc, char* argv[]) {
   dynamic_array_test();
   dynamic_string_test();
   return 0;
}