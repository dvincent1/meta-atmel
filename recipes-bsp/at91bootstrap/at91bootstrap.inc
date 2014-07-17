SUMMARY = "Initial bootstrap for AT91 ARM MPUs"
DESCRIPTION = " \
		at91bootstrap is the second-level bootloader for Atmel AT91  \
		SoCs. It provides a set of algorithms to manage the hardware \
		initialization and to download the main application (or a    \
		third-level bootloader) from specified boot media to         \
		main memory and start it.                                    \
	      "
AUTHOR = "Atmel Corporation"
HOMEPAGE = "http://www.at91.com/linux4sam/bin/view/Linux4SAM/AT91Bootstrap"
BUGTRACKER = "https://github.com/linux4sam/at91bootstrap/issues"
SECTION = "bootloaders"
LICENSE = "ATMEL"
LIC_FILES_CHKSUM = "file://main.c;endline=27;md5=42f86d2f6fd17d1221c5c651b487a07f"

inherit cml1 deploy

COMPATIBLE_MACHINE = "(at91sam9x|sama5d3x)"

AT91BOOTSTRAP_MACHINE ??= "${MACHINE}"
AT91BOOTSTRAP_TARGET ??= "${AT91BOOTSTRAP_MACHINE}nf_uboot_defconfig"

AT91BOOTSTRAP_SUFFIX ?= "bin"
AT91BOOTSTRAP_IMAGE ?= "at91bootstrap-${MACHINE}-${PV}-${PR}.${AT91BOOTSTRAP_SUFFIX}"
AT91BOOTSTRAP_BINARY ?= "at91bootstrap.${AT91BOOTSTRAP_SUFFIX}"
AT91BOOTSTRAP_SYMLINK ?= "at91bootstrap-${MACHINE}.${AT91BOOTSTRAP_SUFFIX}"

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} CC="${TARGET_PREFIX}gcc ${TOOLCHAIN_OPTIONS}"'

do_configure() {
	# Copy board defconfig to .config if .config does not exist. This
	# allows recipes to manage the .config themselves in
	# do_configure_prepend().
	if [ -f "${S}/board/${AT91BOOTSTRAP_MACHINE}/${AT91BOOTSTRAP_TARGET}" ] && [ ! -f "${B}/.config" ]; then
		cp "${S}/board/${AT91BOOTSTRAP_MACHINE}/${AT91BOOTSTRAP_TARGET}" "${B}/.config"
	fi

	# Copy defconfig to .config if .config does not exist. This allows
	# recipes to manage the .config themselves in do_configure_prepend()
	# and to override default settings with a custom file.
	if [ -f "${WORKDIR}/defconfig" ] && [ ! -f "${B}/.config" ]; then
		cp "${WORKDIR}/defconfig" "${B}/.config"
	fi

	if [ ! -f "${S}/.config" ]; then
		bbfatal "No config files found"
	fi

	cml1_do_configure
}

do_compile() {
	if [ "${@base_contains('DISTRO_FEATURES', 'ld-is-gold', 'ld-is-gold', '', d)}" = "ld-is-gold" ] ; then
		sed -i 's/$(CROSS_COMPILE)ld$/$(CROSS_COMPILE)ld.bfd/g' ${S}/Makefile
	fi

	unset CFLAGS CPPFLAGS LDFLAGS
	oe_runmake
}

do_deploy () {
	install -d ${DEPLOYDIR}
	install ${S}/binaries/${AT91BOOTSTRAP_BINARY} ${DEPLOYDIR}/${AT91BOOTSTRAP_IMAGE}

	cd ${DEPLOYDIR}
	rm -f ${AT91BOOTSTRAP_BINARY} ${AT91BOOTSTRAP_SYMLINK}
	ln -sf ${AT91BOOTSTRAP_IMAGE} ${AT91BOOTSTRAP_SYMLINK}
	ln -sf ${AT91BOOTSTRAP_IMAGE} ${AT91BOOTSTRAP_BINARY}

	# Create a symlink ready for file copy on SD card
	rm -f boot.bin
	ln -sf ${AT91BOOTSTRAP_IMAGE} boot.bin
}
addtask deploy before do_build after do_compile