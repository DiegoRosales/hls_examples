SUMMARY = "Enable systemd-resolved mDNS so the board is reachable as <hostname>.local"
DESCRIPTION = "Ships two systemd drop-ins that turn on MulticastDNS: one global \
(resolved.conf.d) and one per-link on the wired interface (80-wired.network.d). \
Without these, resolved neither answers nor resolves .local names, so e.g. \
'ping amd-edf.local' fails. Equivalent to 'resolvectl mdns end0 yes', persisted."
LICENSE = "CLOSED"

SRC_URI = " \
    file://resolved-mdns.conf \
    file://network-mdns.conf \
"

S = "${WORKDIR}"

# The mDNS responder/resolver is systemd-resolved; the per-link drop-in targets
# the networkd-managed wired interface.
RDEPENDS:${PN} += "systemd"

do_install() {
    # Global default: turn MulticastDNS on for resolved.
    install -d ${D}${sysconfdir}/systemd/resolved.conf.d
    install -m 0644 ${WORKDIR}/resolved-mdns.conf \
        ${D}${sysconfdir}/systemd/resolved.conf.d/mdns.conf

    # Per-link: enable mDNS on the wired interface. The vendor unit
    # /usr/lib/systemd/network/80-wired.network is read-only, so override via
    # its .d drop-in directory.
    install -d ${D}${sysconfdir}/systemd/network/80-wired.network.d
    install -m 0644 ${WORKDIR}/network-mdns.conf \
        ${D}${sysconfdir}/systemd/network/80-wired.network.d/mdns.conf
}

FILES:${PN} += " \
    ${sysconfdir}/systemd/resolved.conf.d/mdns.conf \
    ${sysconfdir}/systemd/network/80-wired.network.d/mdns.conf \
"
