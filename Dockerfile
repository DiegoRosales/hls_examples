# Use the official Ubuntu Jammy base image
FROM ubuntu:focal

# Update package lists and install required packages
RUN apt-get update -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y locales libtinfo-dev libtinfo5 lsb-release sudo \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common

RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y libncurses5-dev
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y libncursesw5-dev
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y libtool
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y net-tools
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y autoconf
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y xterm texinfo 
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y texinfo 
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y gcc-multilib
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y gawk
RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y zlib1g zlib1g-dev build-essential

RUN dpkg --add-architecture i386 \
    && apt-get update -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y zlib1g:i386 zlib1g-dev:i386


RUN DEBIAN_FRONTEND=noninteractive apt-get install -y wget
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y bc
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libtool-bin
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y gperf
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y bison
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y flex
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y texi2html
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y texinfo
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y help2man
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y gawk
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libtool
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y automake
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libglib2.0-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libfdt-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y device-tree-compiler
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y qemu-user-static
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y binfmt-support
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y multistrap
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y lib32z1
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y lib32stdc++6
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libssl-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y kpartx
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y dosfstools
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y nfs-common
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y zerofree
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y u-boot-tools
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y rpm2cpio
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y curl
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libsdl1.2-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libpixman-1-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libc6-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y chrpath
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y socat
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y zlib1g-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y zlib1g:i386
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y unzip
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y rsync
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3-pip
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y gcc-multilib
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y xterm
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y net-tools
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libidn11
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y ninja-build
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3-testresources

# Generate the en_US.UTF-8 locale
RUN locale-gen en_US.UTF-8

# Set the default locale to en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8

# Create a new user
RUN useradd -m diego

# Set a password for the new user (replace 'mypassword' with your desired password)
RUN echo 'diego:mypassword' | chpasswd

# Add the new user to the sudo group to grant sudo privileges
RUN usermod -aG sudo diego

# Allow the user to run sudo without entering a password (optional, for convenience)
RUN echo 'diego ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers.d/diego-nopasswd

# You can add more instructions and configurations as needed
# Copy the setup_host.sh script to the container
COPY submodules/PYNQ/sdbuild/scripts/setup_host.sh /tmp/setup_host.sh

# Make the script executable (if needed)
# RUN chmod +x /tmp/setup_host.sh

# Run the script as the root user
RUN /bin/bash /tmp/setup_host.sh


RUN DEBIAN_FRONTEND=noninteractive apt-get install -y qemu-system

RUN wget -O /usr/bin/qemu-x86_64-static https://github.com/multiarch/qemu-user-static/releases/download/v5.2.0-1/qemu-x86_64-static
RUN wget -O /usr/bin/qemu-aarch64-static https://github.com/multiarch/qemu-user-static/releases/download/v5.2.0-1/qemu-aarch64-static
RUN wget -O /usr/bin/qemu-arm-static https://github.com/multiarch/qemu-user-static/releases/download/v5.2.0-1/qemu-arm-static

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git

# Install cg-ng
RUN wget -O /tmp/crosstool-ng-1.26.0_rc1.tar.xz http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.26.0_rc1.tar.xz
RUN cd /tmp && tar -xf crosstool-ng-1.26.0_rc1.tar.xz
RUN cd /tmp/crosstool-ng-1.26.0_rc1 && ./configure --enable-local && make && make install

RUN chmod 777 /tmp
RUN chsh -s /bin/bash diego
# Switch to the new user
USER diego

#COPY ./submodules/PYNQ /tmp/PYNQ
RUN git clone https://github.com/Xilinx/PYNQ.git /tmp/PYNQ
COPY ./submodules/ZYBO-PYNQ /tmp/PYNQ/boards/Zybo

COPY pynq_sdist.tar.gz /tmp/PYNQ/sdbuild/prebuilt/
COPY pynq_rootfs.arm.tar.gz /tmp/PYNQ/sdbuild/prebuilt/

COPY scripts/makefile /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/makefile
# COPY target/fft_demo_integ/fft_demo_top_wrapper.hdf /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.hdf
# COPY target/fft_demo_integ/fft_demo_top_wrapper.xsa /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.xsa
# COPY target/fft_demo_integ/fft_demo_integ.runs/impl_1/fft_demo_top_wrapper.bit /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.bit

RUN sudo chmod 777 -R /tmp/PYNQ

RUN git config --global --add safe.directory /tmp/PYNQ


# Set the working directory to the user's home directory
WORKDIR /home/diego
COPY entrypoint.bash /home/diego/entrypoint.bash


# Start a shell or your desired application when the container is run
# CMD ["/bin/bash"]
SHELL ["/bin/bash", "-c"]