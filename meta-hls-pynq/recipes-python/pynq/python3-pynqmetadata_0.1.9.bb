SUMMARY = "PYNQ Metadata - pydantic models describing a PYNQ overlay"
HOMEPAGE = "https://github.com/Xilinx/PynqMetadata"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4d2124d4ae21e284f8fab7ae4d5dd882"

SRC_URI[sha256sum] = "14fe64f71c7729465aebb43b8e3aabaa05d4286885b2bb2cb01451360d42bc6f"

inherit pypi setuptools3

RDEPENDS:${PN} += " \
    python3-jsonschema \
    python3-pydantic \
    python3-ipython \
"

# RISK: pynqmetadata 0.1.9 pins 'pydantic==1.9.1' (pydantic v1); meta-python
# ships pydantic 2.7.4 (v2), which has breaking API changes, and pydantic 1.9.1
# itself predates Python 3.12 (our target). If `import pynqmetadata` fails at
# runtime, add a pydantic v1 recipe (1.10.x supports 3.12) and PREFERRED_VERSION
# it. See STAGE2-PYNQ-PLAN.md.
