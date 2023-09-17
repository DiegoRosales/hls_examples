cd submodules/PYNQ/boards
ln -s ../../ZYBO-PYNQ Zybo
cd -
cp scripts/makefile submodules/ZYBO-PYNQ/petalinux_bsp/hardware_project/
cp target/fft_demo_integ/fft_demo_top_wrapper.hdf submodules/ZYBO-PYNQ/petalinux_bsp/hardware_project/zybo.hdf
cp target/fft_demo_integ/fft_demo_top_wrapper.xsa submodules/ZYBO-PYNQ/petalinux_bsp/hardware_project/zybo.xsa
cp target/fft_demo_integ/fft_demo_integ.runs/impl_1/fft_demo_top_wrapper.bit submodules/ZYBO-PYNQ/petalinux_bsp/hardware_project/zybo.bit
