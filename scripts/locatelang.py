import numpy as np
import os


def low_pass_filter(signal, frequency_dropoff=1e4):
    out = np.fft.fft(signal)
    freq = np.fft.fftfreq(signal.size)
    return np.fft.ifft(out * np.e**-np.abs(frequency_dropoff * freq)).real


results_folder = os.path.join('..', 'results')
# information_bins = os.listdir(results_folder)
information_bins = ['lang_en_en+pt.bin', 'lang_pt_en+pt.bin']

data_row = np.fromfile(os.path.join(results_folder, information_bins[0]), np.float64)
data_mtx = np.zeros((len(information_bins), data_row.size))
data_mtx[0, :] = low_pass_filter(data_row)

data_to_filename = {'0': information_bins[0]}

for i, information_bin in enumerate(information_bins[1:]):

    data_to_filename[str(i+1)] = information_bin

    data_row = np.fromfile(os.path.join(results_folder, information_bin), np.float64)

    data_mtx[i + 1, :] = low_pass_filter(data_row)

global_threshold = 2

minimum_references = np.argmin(data_mtx, axis=0)

minimums_to_consider = np.min(data_mtx, axis=0) < global_threshold

minimum_references[~minimums_to_consider] = -1

# nan nan nan 9 9 9 9 nan nan nan nan nan 7 7 nan nan

prev = np.arange(minimum_references.size)
prev[~minimums_to_consider] = -1
prev = np.maximum.accumulate(prev)

filled = minimum_references[prev]
first_non_minus1 = np.argwhere(minimum_references != -1)[0][0]
filled[:first_non_minus1] = filled[first_non_minus1]

sections = np.hstack(([0], np.argwhere(np.ediff1d(filled) != 0).flatten() + 1))

print(sections)
print(filled[sections])

# Generate contiguous intervals
intervals = []
start = None

for i in range(len(minimum_references)-1):
    if minimum_references[i] == minimum_references[i+1]:
        if start is None:
            start = i
    else:
        if start is not None:
            intervals.append((start, i+1, data_to_filename[str(minimum_references[start])] if minimum_references[start] != -1 else None))
            start = None

# Check if an interval is open at the end of the array
if start is not None:
    intervals.append((start, len(minimum_references),data_to_filename[str(minimum_references[start])] if minimum_references[start] != -1 else None))

# Print intervals
for interval in intervals:
    print(interval)





