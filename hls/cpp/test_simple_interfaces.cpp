#include "ap_int.h"

void test_simple_interfaces(const ap_uint<2> in1,
                            const ap_uint<2> in2,

                            ap_uint<4> &out) {

  out = in1 * in2;
}