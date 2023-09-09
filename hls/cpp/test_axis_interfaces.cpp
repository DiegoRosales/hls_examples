#include "ap_int.h"
#include "hls_stream.h"

void test_axis_interfaces(hls::stream<ap_uint<2>> &in1,
                            hls::stream<ap_uint<2>> &in2,

                            hls::stream<ap_uint<4>> &out) {

  out.write(in1.read() * in2.read());
}
