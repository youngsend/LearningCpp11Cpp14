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

  return EXIT_SUCCESS;
}