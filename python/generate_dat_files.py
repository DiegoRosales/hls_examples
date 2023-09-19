# %% Import libraries
import numpy as np
from scipy.io import wavfile

# %% Set sample audio file
wav_dir = "../misc/"
wav_filename = "file_example_WAV_1MG"
wav_path = wav_dir + wav_filename + ".wav"

# Read the WAV file
sample_rate, audio_data = wavfile.read(wav_path)

# If audio_data is a stereo array (2D), split it into left and right channels
if len(audio_data.shape) == 2:
    mono_channel = (audio_data[:, 0] + audio_data[:, 1])
else:
    mono_channel = audio_data  # Handle mono audio as well

# Export wav file as a dat file
# Add an index column to the data
data_with_index = np.column_stack((np.arange(len(mono_channel)), mono_channel))
# Define the file path where you want to save the DAT file
file_path = "../dat/" + wav_filename + ".dat"
# Export the array to the DAT file with each element on a new line
np.savetxt(
    file_path,
    data_with_index,
    fmt="%d %d",
    delimiter="\n",
    comments="",
)

# %% Generate dat file for a 20 kHz sine wave
N = 4096
t = np.arange(N) / sample_rate
f = 20e3
sine_20khz_int = np.array(np.sin(2 * np.pi * f * t) * 2**15, dtype=np.int16)
sine_20khz = sine_20khz_int / 2**15
# Export wav file as a dat file
# Add an index column to the data
data_with_index = np.column_stack((np.arange(len(sine_20khz)), sine_20khz))
# Define the file path where you want to save the DAT file
file_path = "../dat/" + "sin20khz" + ".dat"
# Export the array to the DAT file with each element on a new line
np.savetxt(
    file_path,
    data_with_index,
    fmt="%d %lf",
    delimiter="\n",
    comments="",
)

# %% Generate dat file for a 1 kHz sine wave
N = 4096
t = np.arange(N) / sample_rate
f = 1e3
sine_1khz_int = np.array(np.sin(2 * np.pi * f * t) * 2**15, dtype=np.int16)
sine_1khz = sine_1khz_int / 2**15
# Export wav file as a dat file
# Add an index column to the data
data_with_index = np.column_stack((np.arange(len(sine_1khz)), sine_1khz))
# Define the file path where you want to save the DAT file
file_path = "../dat/" + "sin1khz" + ".dat"
# Export the array to the DAT file with each element on a new line
np.savetxt(
    file_path,
    data_with_index,
    fmt="%d %lf",
    delimiter="\n",
    comments="",
)
# %%
