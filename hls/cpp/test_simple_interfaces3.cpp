#include "ap_int.h"

void test_simple_interfaces3(const ap_uint<2> in1,
                             const ap_uint<2> in2,
                             const ap_uint<10> in3,
                             const ap_uint<10> in4,

                             ap_uint<20> &out1,
                             ap_uint<20> &out2) {

  out1 = in1 * in2;
  out1 = in1 * in2 + in3 + in4;
}