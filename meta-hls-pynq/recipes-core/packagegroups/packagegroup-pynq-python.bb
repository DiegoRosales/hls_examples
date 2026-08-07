SUMMARY = "Python runtime dependencies for PYNQ notebooks (plus matplotlib)"
DESCRIPTION = "Mirrors the RDEPENDS from PYNQ's python-pynq.inc, mapped to \
scarthgap package names, plus matplotlib. Installed into the image so notebooks \
have the PYNQ runtime dependencies available."

# This group pulls in native libraries (libdrm, lmsensors-libsensors) that the
# debian class dynamically renames (-> libdrm2, libsensors5). An allarch
# packagegroup may not depend on renamed packages, so make it machine-specific.
# NOTE: this MUST be set before 'inherit packagegroup' -- the class snapshots
# PACKAGE_ARCH with ':=' at inherit time to decide whether to inherit allarch,
# so setting it afterwards is too late (the recipe stays allarch).
PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS:${PN} = "\
    python3-core \
    python3-asyncio \
    python3-cffi \
    python3-json \
    python3-math \
    python3-mmap \
    python3-multiprocessing \
    python3-numpy \
    python3-pycparser \
    python3-resource \
    python3-setuptools \
    python3-threading \
    python3-xml \
    python3-matplotlib \
    lmsensors-libsensors \
    libdrm \
"

# python-pynq.inc also lists python3-importlib, python3-re, python3-signal and
# python3-subprocess. In scarthgap those stdlib modules are folded into
# python3-core, so they are intentionally omitted here -- listing them as
# separate packages would fail the build with "Nothing RPROVIDES".
