SUMMARY = "PYNQ Utils - build/runtime utilities for PYNQ"
HOMEPAGE = "https://github.com/Xilinx/PynqUtils"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4d2124d4ae21e284f8fab7ae4d5dd882"

SRC_URI[sha256sum] = "712d357fdd626e98acd404b5d69cdf58f32b848912d1b9bbd49a6dd2ce60e6bb"

inherit pypi setuptools3

RDEPENDS:${PN} += " \
    python3-pynqmetadata \
    python3-cffi \
    python3-tqdm \
    python3-numpy \
    python3-magic \
"

# pynqutils' setup.py also lists 'python-magic-bin' (a Windows/macOS binary
# wheel bundling libmagic) -- intentionally omitted. On Linux, python3-magic
# uses the system libmagic from the 'file' recipe. setup.py version pins
# (setuptools<=80, numpy<2.0) are advisory only; Yocto resolves deps via
# RDEPENDS above.
