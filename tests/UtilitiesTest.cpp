#include "utilities/Common.hpp"

#include <stdexcept>

int life_notifier_comp(const int* A, const int* B) {
   return *A - *B;
}

int dynamic_array_test() {
    ctDebugLog("Dynamic Array");
    ctDebugWarning("Warning test");
   {
      Ct::DynamicArray<int> arr = {};
      /*Reserve*/
      ctDebugLog("Reserve");
      arr.reserve(64);
      /*Memset*/
      ctDebugLog("Memset");
      arr.setbytes(0);
      /*Append*/
      ctDebugLog("Append");
      for (int i = 0; i < 32; i++) {
         arr.append(32 - i);
      }
      /*Insert*/
      ctDebugLog("Insert");
      arr.insert(-32, 0);
      arr.insert(-64, 8);
      arr.insert(-128, -2);
      /*Remove*/
      ctDebugLog("Remove");
      arr.remove(-2);
      arr.remove(8);
      arr.remove(0);
      /*Exists*/
      ctDebugLog("Exists");
      if (arr.exists(4)) { ctDebugLog("Found 4"); }
      /*Find*/
      ctDebugLog("Find");
      int* ln = NULL;
      ln = arr.findptr(12, 0, 1);
      ln = arr.findptr(7, -1, -1);
      /*Sort*/
      ctDebugLog("Sort");
      arr.sort(0, arr.count(), life_notifier_comp);
      /*Hash*/
      ctDebugLog("Hash");
      /*Verify*/
      ctDebugLog("Verify");
      int previous = arr[0];
      for (int i = 1; i < arr.count(); i++) {
         if (arr[i] - 1 != previous) { return -1; }
         previous = arr[i];
      }
      /*End*/
      arr.clear();
      ctDebugLog("End");
   }
   return 0;
}

int dynamic_string_test()
{
    Ct::StringUtf8 mystring = "String";
    return 0;
}

int main(int argc, char* argv[]) {
   return dynamic_array_test();
}