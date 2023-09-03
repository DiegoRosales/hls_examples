# %% Import libraries
import numpy as np
from scipy.io import wavfile
import matplotlib.pyplot as plt
%matplotlib widget

# %% Floating point to fixed point representation converter
def float_to_fixed_point(floating_point_num, decimal_bits=15):
    # Define the scaling factor based on the decimal bits
    scaling_factor = 2 ** decimal_bits

    # Convert the floating-point number to fixed-point
    fixed_point_num = int(floating_point_num * scaling_factor)

    return fixed_point_num


# %% Set sample audio file
wav_file = '../misc/file_example_WAV_1MG.wav'

# %% Read the WAV file
sample_rate, audio_data = wavfile.read(wav_file)

# %% If audio_data is a stereo array (2D), split it into left and right channels
if len(audio_data.shape) == 2:
    left_channel = audio_data[:, 0]
    right_channel = audio_data[:, 1]
else:
    mono_channel = audio_data  # Handle mono audio as well

# %% Convert the channel data to NumPy arrays
left_array = np.array(left_channel, dtype=np.int16)
right_array = np.array(right_channel, dtype=np.int16)

# %% Print some information about the WAV file
print(f"Sample rate: {sample_rate}")
print(f"Number of samples per channel: {left_array.shape[0]}")
print(f"Duration (seconds) per channel: {left_array.shape[0] / sample_rate}")

# %% Plot both channels signals
t = np.arange(left_array.size)/sample_rate
fig, axs = plt.subplots(2, 1, constrained_layout=True, figsize=(10, 8))
axs[0].plot(t, left_array)
axs[0].set_title("Left channel")
axs[0].set_xlabel("Time [s]")
axs[0].set_ylabel("Amplitude")
axs[1].plot(t, right_array)
axs[1].set_title("Right channel")
axs[1].set_xlabel("Time [s]")
axs[1].set_ylabel("Amplitude")
plt.show()

# %%
def dft_magnitude(input_signal, N=1024):
    twiddle_factors = [np.exp(-2.0j * np.pi * k / N) for k in range(N)]
    dft_result = np.zeros(N//2,dtype=complex)
    for k in range(N//2):
        for n in range(N):
            dft_result[k] += input_signal[n] * np.real(twiddle_factors[(k * n) % N]) + 1j* input_signal[n] * np.imag(twiddle_factors[(k * n) % N])
    dft_magnitude = np.abs(dft_result)
    return dft_magnitude

# %% Test algorithm with sample wave
N = 1024
spectrum = dft_magnitude(left_array[0:N], N)
f = np.fft.fftfreq(N, 1/sample_rate)
plt.figure()
plt.plot(f[0:spectrum.size],spectrum)
plt.title("Hand made")
plt.show()
# %%
N = 1024
spectrum2 = np.fft.fft(left_array[0:N])
f = np.fft.fftfreq(N, 1/sample_rate)
plt.figure()
plt.plot(f[0:spectrum.size], np.abs(spectrum2[0:spectrum.size]))
plt.title("Numpy")
plt.show()

# %% Test algorithm with 20 kHz sine wave
N = 4096
t = np.arange(N)/sample_rate
sine_20khz = np.array(np.sin(2*np.pi*20e3*t)*2**15,dtype=np.int16)
spectrum = dft_magnitude(sine_20khz, N)
#%%
f = np.fft.fftfreq(N, 1/sample_rate)
plt.figure()
plt.plot(f[0:spectrum.size],spectrum)
plt.title("Hand made")
plt.show()

# %%
