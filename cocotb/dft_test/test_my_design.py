# test_my_design.py (extended)

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

CLK_PERIOD_NS = 10


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


@cocotb.test()
async def my_second_test(dut):
    ## Read an AXI Lite address
    async def read_reg(ADDRESS):
        task = cocotb.start_soon(axim.read(ADDRESS))
        timeout = Timer(CLK_PERIOD_NS * 20, units="ns")
        result = await First(task, timeout)
        if result is timeout:
            dut._log.error("Timeout!")
            return None
        else:
            dut._log.debug(f"REG[{ADDRESS:02x}] = 0x{result.integer:x}")
            return result.integer

    dut._log.info("Starting testcase")

    ## AXI4-Lite interface
    dut._log.info("Setting up AXI4 Lite interface")
    axi_lite_prefix = ""
    for i in inspect.getmembers(dut):
        if i[0].endswith("_WDATA"):
            print(f'Found AXI4-Lite: {i[0].split("_WDATA")[0]}')
            axi_lite_prefix = i[0].split("_WDATA")[0]
    dut._log.info(f"AXI Lite = {axi_lite_prefix}")
    axim = AXI4LiteMaster(dut, axi_lite_prefix, dut.ap_clk)

    await setup_dut(dut)
    start_time = get_sim_time(units="ns")
    dut._log.info(f"Start time = {start_time}ns")
    for i in range(10):
        dut._log.info(f"Reading = {get_sim_time(units='ns')}ns")
        result = await read_reg(0)
        dut._log.info(f"Reading = {get_sim_time(units='ns')}ns")
    dut._log.info(f"End time = {get_sim_time(units='ns')}ns")
    dut._log.info(f"Duration = {get_sim_time(units='ns') - start_time}ns")
    if result is not None:
        dut._log.info(f"Result = {hex(result)}")
    else:
        dut._log.error(f"Result is None!")
    dut._log.info("Test done")
