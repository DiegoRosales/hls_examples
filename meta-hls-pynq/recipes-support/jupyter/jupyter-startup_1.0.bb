SUMMARY = "Autostart Jupyter Lab at boot, serving /home/root/notebooks"
DESCRIPTION = "Glue recipe that creates the notebook directory and installs a \
sysvinit script and systemd unit to launch Jupyter Lab on 0.0.0.0:8888."
LICENSE = "CLOSED"

SRC_URI = " \
    file://jupyter.service \
    file://jupyter.init \
"

S = "${WORKDIR}"

# Jupyter itself comes from meta-jupyter; bash is needed for the login shell.
RDEPENDS:${PN} += " \
    packagegroup-python3-jupyter \
    bash \
"

NOTEBOOK_DIR ?= "/home/root/notebooks"

# Ship both init flavours; each class self-gates on DISTRO_FEATURES, so only
# the one matching the active init manager actually takes effect.
inherit update-rc.d systemd

INITSCRIPT_NAME = "jupyter"
INITSCRIPT_PARAMS = "defaults 99"

SYSTEMD_SERVICE:${PN} = "jupyter.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
    install -d ${D}${NOTEBOOK_DIR}

    # systemd unit
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/jupyter.service ${D}${systemd_system_unitdir}/jupyter.service

    # sysvinit script
    install -d ${D}${INIT_D_DIR}
    install -m 0755 ${WORKDIR}/jupyter.init ${D}${INIT_D_DIR}/jupyter
}

FILES:${PN} += "${NOTEBOOK_DIR}"
