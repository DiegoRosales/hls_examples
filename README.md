# hls_examples

## Run HLS
```
cd hls
vitis_hls -f run_hls.tcl fft_wrapper
```
## Prepare the project
```
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK"
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "INTEG"
```

## Open Vivado GUI
vivado target/fft_demo_integ/fft_demo_integ.xpr

## Setup docker for Pynq
git submodule update --init
cd submodules/PYNQ/boards
ln -s ../../ZYBO-PYNQ Zybo
cd -
cp scripts/makefile submodules/ZYBO-PYNQ/petalinux_bsp/hardware_project/

wget -O pynq_rootfs.arm.tar.gz https://bit.ly/pynq_arm_v3_1
wget -O pynq_sdist.tar.gz https://bit.ly/pynq_sdist_v3_0_1

sudo apt-get install qemu-user-static
sudo update-binfmts --enable qemu-arm

docker build -t ubuntu-focal-configured .

docker run -it -v $(pwd):/tmp/output -v $(pwd)/target/fft_demo_integ:/tmp/input:ro -v /unraid:/unraid --cap-add=SYS_ADMIN --cap-add=SYS_CHROOT --cap-add=NET_ADMIN --security-opt systempaths=unconfined --security-opt apparmor=unconfined --privileged ubuntu-focal-configured

# Inside docker
source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh ; source /unraid/tools/PetaLinux/2022.1/settings.sh ; ./scripts/setup_host.sh
git clone https://github.com/besser82/libxcrypt.git /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads/git2/github.com.besser82.libxcrypt.git
wget -P /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads http://www.xmlsoft.org/sources/libxml2-2.9.12.tar.gz
wget -P /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
wget -P /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads https://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.gz
wget -P /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads https://www.cpan.org/src/5.0/perl-5.34.0.tar.gz


make BOARDS=Zybo
cp -rf /tmp/PYNQ/sdbuild/output /tmp/output/