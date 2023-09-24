# %% imports

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import Timer, First, Combine
from cocotb.utils import get_sim_time
from cocotb_bus.drivers.amba import AXI4LiteMaster, AXIProtocolError
from cocotbext.axi import (
    AxiStreamFrame,
    AxiStreamBus,
    AxiStreamSource,
    AxiStreamSink,
    AxiStreamMonitor,
)
import inspect
import logging
import numpy as np
from scipy.io import wavfile
import matplotlib.pyplot as plt

CLK_PERIOD_NS = 10

# %% Setup sine wave

sample_rate = 44100
N = 256
t = np.arange(N) / sample_rate
f = 20e3
sine_20khz_int = np.array(np.sin(2 * np.pi * f * t) * 2**15, dtype=np.int16)

N_RTL = 256
FIXED_POINT_BITS = 24
# %% TB Utilities

async def generate_clock(dut):
    """Generate clock pulses."""

    for cycle in range(10):
        dut.clk.value = 0
        await Timer(1, units="ns")
        dut.clk.value = 1
        await Timer(1, units="ns")


async def setup_dut(dut):
    dut._log.info("------------------")
    dut._log.info("Starting DUT setup...")

    ## Assert resets
    dut._log.info("Deasserting resets")
    dut.ap_rst_n.value = 1
    #    dut.ap_rst_n_ctrl_clk.value = 1

    ## Start the clocks
    dut._log.info("Starting clocks...")
    #    cocotb.start_soon(Clock(dut.ctrl_clk, CLK_PERIOD_NS, units="ns").start())
    cocotb.start_soon(Clock(dut.ap_clk, CLK_PERIOD_NS, units="ns").start())

    ## Wait 10 clock cycles
    dut._log.info(
        f"Waiting {CLK_PERIOD_NS * 10}ns | SIM_TIME = {get_sim_time(units='ns')}"
    )
    await Timer(CLK_PERIOD_NS * 10, units="ns")

    ## Assert resets
    dut._log.info("Asserting resets")
    dut.ap_rst_n.value = 0
    #    dut.ap_rst_n_ctrl_clk.value = 0

    ## Wait 10 clock cycles
    dut._log.info(
        f"Waiting {CLK_PERIOD_NS * 10}ns | SIM_TIME = {get_sim_time(units='ns')}"
    )
    await Timer(CLK_PERIOD_NS * 10, units="ns")

    ## Deassert resets
    dut._log.info(f"Deasserting resets | SIM_TIME = {get_sim_time(units='ns')}")
    dut.ap_rst_n.value = 1
    #    dut.ap_rst_n_ctrl_clk.value = 1

    ## Wait 10 clock cycles
    dut._log.info(
        f"Waiting {CLK_PERIOD_NS * 10}ns | SIM_TIME = {get_sim_time(units='ns')}"
    )
    await Timer(CLK_PERIOD_NS * 10, units="ns")

    dut._log.info("DUT setup complete!")
    dut._log.info("------------------")


def int_to_fixed(value:int, decimals:int) -> float:
    return float(value) / float(2**decimals)

def fixed_to_int(value:float, decimals:int):
    return int(value * (2**decimals))

# %% Test
@cocotb.test()
async def test_fft(dut):
    # %% Read an AXI Lite address
    async def read_reg(ADDRESS):
        task = cocotb.start_soon(axim.read(ADDRESS))
        timeout = Timer(CLK_PERIOD_NS * 20, units="ns")
        result = await First(task, timeout)
        if result is timeout:
            dut._log.error("Timeout!")
            return None
        else:
            # dut._log.info(f"REG[0x{ADDRESS:02x}] = 0x{result.integer:x}")
            return result.integer
    # %% Write an AXI Lite address
    async def write_reg(ADDRESS, DATA):
        task = cocotb.start_soon(axim.write(ADDRESS, DATA))
        timeout = Timer(CLK_PERIOD_NS * 20, units="ns")
        result = await First(task, timeout)
        if result is timeout:
            dut._log.error("Timeout!")
        dut._log.info(f"REG[0x{ADDRESS:02x}] <= 0x{DATA:x}")
        return None
    
    # %% Find the axi-l slave
    def find_axi_master(dut):
        dut._log.info("Setting up AXI4 Lite interface")
        axi_lite_prefix = ""
        for i in inspect.getmembers(dut):
            if i[0].endswith("_WDATA"):
                print(f'Found AXI4-Lite: {i[0].split("_WDATA")[0]}')
                axi_lite_prefix = i[0].split("_WDATA")[0]
        dut._log.info(f"AXI Lite = {axi_lite_prefix}")
        axim = AXI4LiteMaster(dut, axi_lite_prefix, dut.ap_clk)

        return axim

    # %% Start the test
    dut._log.info("Starting testcase")

    ## AXI4-Lite interface
    axim = find_axi_master(dut)

    ## AXI-S interfaces
    input_stream = AxiStreamSource(AxiStreamBus.from_prefix(dut, "input_signal"), dut.ap_clk, dut.ap_rst_n, reset_active_level=False, byte_lanes=1)
    input_stream.log.setLevel(logging.INFO)
    # %% Setup the DUT
    ## Start clocks and stuff
    await setup_dut(dut)
    
    await write_reg(0x0, 0x81)
    result = await read_reg(0)
    if (result != 0x81):
        dut._log.error(f"DUT hasn't started! Result is {hex(result)}")
    else:
        dut._log.info(f"DUT has started! Result is {hex(result)}")
        await Timer(CLK_PERIOD_NS*100, units='ns')
        dut._log.info(f"----------------------------------------")

        
    ########## Test starts here ################

    golden_data = []

    # %% Write data through AXI-S interface
    dut._log.info(f"Sending {sine_20khz_int.size} samples")
    for i in range(sine_20khz_int.size):
        dut._log.info(f"Sending sample {i}/{sine_20khz_int.size}")
        real_values = []
        imag_values = []
        # data_float = 1.0/(1+i)
        # data = fixed_to_int(data_float, 16)
        data_float = int(sine_20khz_int[i])
        data = data_float
        golden_data.append(data)
        dut._log.info(f"Sending {data_float} as {data}")
        timeout = Timer(CLK_PERIOD_NS * 200, units='ns')
        data_list = []
        task = cocotb.start_soon(input_stream.write(AxiStreamFrame([data])))
        res = await First(task, timeout)
        if res == timeout:
            dut._log.error("ERROR - There was a timeout while waiting for the streams to complete")
        else:
            await Timer(CLK_PERIOD_NS*5, units='ns')
        while (dut.input_signal_TREADY.value.integer == 0):
            # dut._log.info("TREADY = 0")
            await Timer(CLK_PERIOD_NS, units='ns')

    # %% Read FFT values through AXI-Lite interface
    for i in range(N_RTL):
        result = int_to_fixed(await read_reg(0x400 + 4*i), FIXED_POINT_BITS)
        real_values.append(result)
        result = int_to_fixed(await read_reg((N_RTL*4) + 0x400 + 4*i), FIXED_POINT_BITS)
        imag_values.append(result)
    
    # %% Plot
    real_values = np.array(real_values)
    imag_values = np.array(imag_values)
    plot_y = np.abs(real_values + 1j*imag_values)
    print(real_values)
    print(imag_values)
    print(plot_y)
    fig = plt.figure(figsize=(6, 3), dpi=150)
    f = np.fft.fftfreq(N_RTL, 1/sample_rate)
    plt.plot(f[0:plot_y.size],plot_y)
    plt.title("Hand made")

    fig.savefig("test.png", dpi=200)
    dut._log.info("Test done")
