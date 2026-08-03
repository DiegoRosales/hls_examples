"""SSM2603 audio-codec driver for the `sampler_codec_controller` PL IP.

Python/PYNQ port of the FreeRTOS baremetal driver in this directory
(`codec_controller_utils.c`, `codec_controller_reg_utils.c`). The controller is
a custom AXI4-Lite IP whose register map and per-bit behaviour are defined in
`rtl/register_unit/codec_registers.sv`. The IP does the I2C to the SSM2603; this
code only pokes controller registers and polls status bits.

Register semantics (from register_params.svh) that matter for the CTRL reg at
offset 0x00 -- every bit ignores written 0s, so we NEVER read-modify-write it,
we just write the single action bit:
  bit0  data_wr   : write 1 to kick a codec write, HW auto-clears (WR1_HW_CLR)
  bit1  data_rd   : write 1 to kick a codec read,  HW auto-clears (WR1_HW_CLR)
  bit2  busy      : read-only, HW driven                          (RO)
  bit3  init_done : write 1 to clear, HW sets                     (RWC1)
  bit4  data_valid: write 1 to clear, HW sets                     (RWC1)
  bit5  missed_ack: write 1 to clear, HW sets                     (RWC1)
  bit31 reset      : write 1 to set, HW clears on init done       (WR1_HW_CLR)

Usage:
    import zynq_cma_device            # must precede pynq (registers /dev/mem dev)
    from pynq import Overlay
    from codec_controller import CodecController

    ol = Overlay('fft_demo_top_wrapper.bit', download=False)  # attach only
    codec = CodecController(ol.sampler_codec_controller)
    codec.init()
"""

import time

# ---- controller AXI4-Lite byte offsets (word address * 4) -----------------
CTRL     = 0x00   # codec_i2c_ctrl_reg
ADDR     = 0x04   # codec_i2c_addr
WR_DATA  = 0x08   # codec_i2c_wr_data
RD_DATA  = 0x0C   # codec_i2c_rd_data  (HW written, resets to 0xcafecafe)

# ---- CTRL register action/status bits -------------------------------------
BIT_WR         = 1 << 0
BIT_RD         = 1 << 1
BIT_BUSY       = 1 << 2
BIT_INIT_DONE  = 1 << 3
BIT_DATA_VALID = 1 << 4
BIT_MISSED_ACK = 1 << 5
BIT_RESET      = 1 << 31

# Writing this clears all three write-1-to-clear status bits in one shot;
# the WR1/RO bits ignore the 0s, so nothing gets triggered.
CLEAR_STATUS   = BIT_INIT_DONE | BIT_DATA_VALID | BIT_MISSED_ACK

# ---- SSM2603 register addresses (SSM2603_codec_registers.h) ---------------
R_LINVOL   = 0x00   # left  ADC input volume
R_RINVOL   = 0x01   # right ADC input volume
R_LHPVOL   = 0x02   # left  DAC/headphone volume
R_RHPVOL   = 0x03   # right DAC/headphone volume
R_ANALOG   = 0x04   # analog audio path
R_DIGITAL  = 0x05   # digital audio path
R_POWER    = 0x06   # power management
R_IFACE    = 0x07   # digital audio interface
R_SAMPLE   = 0x08   # sampling rate
R_ACTIVE   = 0x09   # active
R_RESET    = 0x0F   # software reset

# Output-volume codes: SSM2603 headphone volume 0x79 == 0 dB, 1 LSB == 1 dB.
_DB_OUT_0DB = 0x79


class CodecBusyError(RuntimeError):
    pass


class CodecController:
    def __init__(self, ip, timeout_s=1.0):
        """`ip` is a PYNQ DefaultIP, e.g. ``overlay.sampler_codec_controller``."""
        self.ip = ip
        self.timeout_s = timeout_s

    # -- raw controller register access -------------------------------------
    def _rd(self, off):
        return self.ip.read(off)

    def _wr(self, off, val):
        self.ip.write(off, val & 0xFFFFFFFF)

    def _wait(self, pred, what):
        t0 = time.monotonic()
        while not pred():
            if time.monotonic() - t0 > self.timeout_s:
                raise TimeoutError(f"timeout waiting for {what} "
                                   f"(CTRL=0x{self._rd(CTRL):08x})")

    def _wait_not_busy(self):
        self._wait(lambda: not (self._rd(CTRL) & BIT_BUSY), "busy to clear")

    # -- controller / codec reset -------------------------------------------
    def controller_reset(self):
        """vControllerReset: pulse the reset bit, HW clears it when done."""
        self._wr(CTRL, BIT_RESET)
        self._wait(lambda: not (self._rd(CTRL) & BIT_RESET),
                   "controller reset to complete")

    def codec_reset(self):
        """vCodecReset: toggle the SSM2603 software-reset register."""
        self.codec_wr(R_RESET, 0x00)
        time.sleep(0.01)
        self.codec_wr(R_RESET, 0x01)
        time.sleep(0.01)

    # -- SSM2603 register read / write via the controller -------------------
    def codec_rd(self, addr):
        """ulCodecRd: read one SSM2603 register through the I2C controller."""
        if self._rd(CTRL) & BIT_BUSY:
            raise CodecBusyError("another transaction is in progress")

        self._wr(CTRL, CLEAR_STATUS)          # vClearStatusBits
        self._wr(ADDR, addr)                  # step 1: address
        self._wr(CTRL, BIT_RD)                # step 2: kick the read
        self._wait_not_busy()                 # step 3: transfer done

        # step 3b: wait for valid data, surfacing a missed ACK
        def _valid():
            s = self._rd(CTRL)
            if s & BIT_MISSED_ACK:
                raise RuntimeError("MISSED ACK from codec")
            return bool(s & BIT_DATA_VALID)
        self._wait(_valid, "codec read data valid")

        self._wr(CTRL, BIT_DATA_VALID)        # clear the valid flag
        return self._rd(RD_DATA) & 0xFF       # step 4: data

    def codec_wr(self, addr, data, check=False):
        """ulCodecWr: write one SSM2603 register; optionally read back."""
        if self._rd(CTRL) & BIT_BUSY:
            raise CodecBusyError("another transaction is in progress")

        self._wr(CTRL, CLEAR_STATUS)          # vClearStatusBits
        self._wr(ADDR, addr)                  # step 1: address
        self._wr(WR_DATA, data)               # step 2: data
        self._wr(CTRL, BIT_WR)                # step 3: kick the write
        self._wait_not_busy()                 # step 4: transfer done

        if check:
            rb = self.codec_rd(addr)
            if rb != (data & 0xFF):
                raise RuntimeError(
                    f"readback mismatch @ 0x{addr:02x}: "
                    f"wrote 0x{data & 0xFF:02x}, read 0x{rb:02x}")

    # -- volume helpers (ulSetOutputVolume / ulSetInputVolume) --------------
    def set_output_volume(self, db):
        """Headphone/DAC volume in dB (approx -73..+6); updates both channels."""
        code = (_DB_OUT_0DB + db) & 0x7F
        val = code | (1 << 8)                 # LRHPBOTH: update L+R together
        self.codec_wr(R_LHPVOL, val)

    def set_input_volume(self, code):
        """ADC input volume, raw 6-bit code (0x17 == 0 dB); updates both ch."""
        val = (code & 0x3F) | (1 << 8)        # LRINBOTH; LINMUTE=0
        self.codec_wr(R_LINVOL, val)

    # -- full bring-up sequence (vCodecInit) --------------------------------
    def init(self, verbose=True):
        """Port of vCodecInit: reset, power up, DSP master mode, 44.1 kHz,
        route DAC->output, unmute, set volumes, activate, enable output."""
        log = print if verbose else (lambda *a, **k: None)

        log("Resetting the CODEC...")
        self.codec_reset()

        # Power management (reg 0x06). Start all-off (0xFF), then power up the
        # chip, DAC, ADC, line-in and CLKOUT -> 0x32. Output stays off for now.
        log("Enabling the PM registers...")
        self.codec_wr(R_POWER, 0x32, check=True)

        # Digital audio IF (0x07): Format=DSP mode (0x3), LRP=1, MS=1 (master).
        log("Setting the CODEC as master...")
        self.codec_wr(R_IFACE, 0x53, check=True)

        # Sampling rate (0x08): USB mode, BOSR=1, SR=0x8 -> ~44.1 kHz (256*fs).
        log("Setting USB / 44.1 kHz...")
        self.codec_wr(R_SAMPLE, 0x23, check=True)

        # Analog audio path (0x04): DACSEL=1, bypass enable (mix the line input with the output).
        log("Setting the analog audio path...")
        self.codec_wr(R_ANALOG, 0x18, check=True)

        # Digital audio path (0x05): unmute DAC (DACMU=0), enable ADC HPF.
        log("Removing the mute...")
        val = self.codec_rd(R_DIGITAL)
        val = (val & ~(1 << 3)) | (1 << 0)
        self.codec_wr(R_DIGITAL, val, check=True)

        log("Setting output volume (-15 dB)...")
        self.set_output_volume(-15)

        log("Setting input volume (0 dB)...")
        self.set_input_volume(0x17)

        time.sleep(0.01)

        # Active (0x09): enable the digital core.
        log("Enabling the digital core...")
        self.codec_wr(R_ACTIVE, 0x01, check=True)

        time.sleep(0.1)

        # Power management again: clear OUTPD (bit4) -> enable output. 0x32->0x22.
        log("Enabling the output...")
        self.codec_wr(R_POWER, 0x22, check=True)

        log("CODEC init done.")
