#!/bin/bash
source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh
source /unraid/tools/PetaLinux/2022.1/settings.sh
export SPAM=EGGS
echo "----"
echo "source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh ; source /unraid/tools/PetaLinux/2022.1/settings.sh ; ./scripts/setup_host.sh ; make BOARDS=Zybo"
echo "./scripts/setup_host.sh"
echo "make BOARDS=Zybo"
echo "----"
exec "$@"