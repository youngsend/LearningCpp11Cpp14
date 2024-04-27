#include <stdio.h>
#include <stdlib.h>

int main() {
  // Declarations
  double A[5] = {
    9.0,
    2.9,
    3.E+25,
    .00007,
  };

  // Doing some work
  for (int i = -1; i < 4; ++i) {
    if (i) {
      printf("element %d is %g, \tits square is %g\n", i, A[i+1], A[i+1]*A[i+1]);
    }
  }

  // test for size_t
  size_t size_of_a = 1U;
  size_t size_of_b = 2U;
  int result = size_of_a - size_of_b;
  printf("result is %d\n", result);

  return EXIT_SUCCESS;
}