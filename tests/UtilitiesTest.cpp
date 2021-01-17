#include "utilities/Common.hpp"

#include <stdexcept>

int life_notifier_comp(const int* A, const int* B) {
   return *A - *B;
}

int dynamic_array_test() {
   printf("Dynamic Array\n");
   {
      Ct::DynamicArray<int> arr = {};
      /*Reserve*/
      printf("Reserve\n");
      arr.reserve(64);
      /*Memset*/
      printf("Memset\n");
      arr.setbytes(0);
      /*Append*/
      printf("Append\n");
      for (int i = 0; i < 32; i++) {
         arr.append(32 - i);
      }
      /*Insert*/
      printf("Insert\n");
      arr.insert(-32, 0);
      arr.insert(-64, 8);
      arr.insert(-128, -2);
      /*Remove*/
      printf("Remove\n");
      arr.remove(-2);
      arr.remove(8);
      arr.remove(0);
      /*Exists*/
      printf("Exists\n");
      if (arr.exists(4)) { printf("Found 4\n"); }
      /*Find*/
      printf("Find\n");
      int* ln = NULL;
      ln = arr.findptr(12, 0, 1);
      ln = arr.findptr(7, -1, -1);
      /*Sort*/
      printf("Sort\n");
      arr.sort(0, arr.count(), life_notifier_comp);
      /*Hash*/
      printf("Hash\n");
      /*Verify*/
      printf("Verify\n");
      int previous = arr[0];
      for (int i = 1; i < arr.count(); i++) {
         if (arr[i] - 1 != previous) { return -1; }
         previous = arr[i];
      }
      /*End*/
      arr.clear();
      printf("End\n");
   }
   return 0;
}

int main(int argc, char* argv[]) {
   return dynamic_array_test();
}