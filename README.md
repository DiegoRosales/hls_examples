# hls_examples

## Prepare the project
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK"
vivado -mode gui   -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "INTEG"

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
docker run -it -v $(pwd):$(pwd) -v /unraid:/unraid -w /tmp/PYNQ/sdbuild --cap-add=SYS_ADMIN --cap-add=SYS_CHROOT --security-opt systempaths=unconfined --security-opt apparmor=unconfined --privileged ubuntu-focal-configured
source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh ; source /unraid/tools/PetaLinux/2022.1/settings.sh ; ./scripts/setup_host.sh
git clone https://github.com/besser82/libxcrypt.git /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads/git2/github.com.besser82.libxcrypt.git
wget -t 2 -T 30 --passive-ftp -P /tmp/PYNQ/sdbuild/build/Zybo/petalinux_project/build/downloads 'http://www.xmlsoft.org/sources/libxml2-2.9.12.tar.gz' --progress=dot -v

make BOARDS=Zybo
