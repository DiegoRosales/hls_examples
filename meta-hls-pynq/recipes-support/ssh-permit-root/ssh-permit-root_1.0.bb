SUMMARY = "Permit password-based root SSH login on the EDF image"
DESCRIPTION = "Drops an OpenSSH config fragment into /etc/ssh/sshd_config.d that \
sets 'PermitRootLogin yes'. Without debug-tweaks OpenSSH defaults to \
prohibit-password (key-only), which rejects the baked-in root password over \
SSH. Enabling this lets you scp a rebuilt boot.bin to the board's boot \
partition instead of pulling the SD card. OpenSSH reads sshd_config.d/*.conf \
via an Include at the top of sshd_config, so this value wins over the default."
LICENSE = "CLOSED"

SRC_URI = "file://permit-root.conf"

S = "${WORKDIR}"

# Config only makes sense with the OpenSSH server present.
RDEPENDS:${PN} = "openssh-sshd"

do_install() {
    install -d ${D}${sysconfdir}/ssh/sshd_config.d
    install -m 0644 ${WORKDIR}/permit-root.conf \
        ${D}${sysconfdir}/ssh/sshd_config.d/10-permit-root.conf
}

FILES:${PN} += "${sysconfdir}/ssh/sshd_config.d/10-permit-root.conf"
