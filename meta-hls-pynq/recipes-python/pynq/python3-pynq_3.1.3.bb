SUMMARY = "PYNQ - Python productivity for Zynq"
HOMEPAGE = "http://pynq.io"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b42e39ad2ddbad7e8ad47f3eee6feff5"

# Build from the in-tree PYNQ submodule (repo root/submodules/PYNQ).
# gitsm: PYNQ has its own submodules; usehead uses the checked-out commit.
SRC_URI = "gitsm://${THISDIR}/../../../submodules/PYNQ;protocol=file;usehead=1"
SRCREV = "${AUTOREV}"
S = "${WORKDIR}/git"

inherit setuptools3

# _video C extension (built for the ZYNQ arch) links against libdrm; boost is
# pulled in by the native bits.
DEPENDS += "libdrm boost"

RDEPENDS:${PN} += " \
    python3-pynqmetadata \
    python3-pynqutils \
    python3-nest-asyncio \
    packagegroup-pynq-python \
"

# setup.py reads PYNQ_BUILD_ARCH to decide whether to build the Zynq C
# extensions; armv7l => build them. libcma ships a prebuilt .so per bitness.
PYNQ_ARCH:arm = "armv7l"
PYNQ_ARCH:aarch64 = "aarch64"
CMA_ARCH:arm = "32"
CMA_ARCH:aarch64 = "64"

do_compile:prepend() {
    install -d "${D}/home/root/notebooks"
    export PYNQ_JUPYTER_NOTEBOOKS="${D}/home/root/notebooks"
    export PYNQ_BUILD_ARCH="${PYNQ_ARCH}"
    export PYNQ_BUILD_ROOT="${STAGING_DIR_TARGET}"
}

do_install:prepend() {
    install -d "${D}/home/root/notebooks"
    export PYNQ_JUPYTER_NOTEBOOKS="${D}/home/root/notebooks"
    export PYNQ_BUILD_ARCH="${PYNQ_ARCH}"
    export PYNQ_BUILD_ROOT="${STAGING_DIR_TARGET}"
}

do_install:append() {
    # libcma ships prebuilt libcma.so.32/.64; 'install' just copies the correct
    # one to ${D}/usr/lib/libcma.so and the header to ${D}/usr/include.
    (cd ${S}/sdbuild/packages/libsds/libcma && \
        oe_runmake install CMA_ARCH=${CMA_ARCH} DESTDIR=${D})
    rm -rf ${D}/home/root/notebooks_*
}

# libcma.so is a prebuilt runtime .so dropped straight into ${libdir}.
SOLIBS = ".so"
FILES_SOLIBSDEV = ""
INSANE_SKIP:${PN} = "staticdev already-stripped"
FILES:${PN} += "${libdir}/libcma.so /home/root/notebooks"

# Ship the bundled notebooks as a separate package.
PACKAGES += "${PN}-notebooks"
FILES:${PN}-notebooks = "/home/root/notebooks"

# scarthgap port of PYNQ's langdale python-pynq.inc: builds from the local
# submodule via gitsm, cross-builds the pynq.lib._video C extension against
# libdrm, and installs the prebuilt libcma.so. For a minimal MMIO-only path the
# _video ext could be dropped by not exporting PYNQ_BUILD_ARCH. See PYNQ-NOTES.md.
