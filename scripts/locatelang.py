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

minimum_references[~minimums_to_consider] = -1                                  # -1 -1 -1  9  9  9  9 -1 -1 -1 -1 -1  7  7 -1 -1   base

prev = np.arange(minimum_references.size)                                       #  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15   generate indices
prev[~minimums_to_consider] = -1                                                # -1 -1 -1  3  4  5  6 -1 -1 -1 -1 -1 12 13 -1 -1   set invalid indices to small number
prev = np.maximum.accumulate(prev)                                              # -1 -1 -1  3  4  5  6  6  6  6  6  6 12 13 13 13   accumulate maximum from left to right

filled = minimum_references[prev]                                               #  ?  ?  ?  9  9  9  9  9  9  9  9  9  7  7  7  7   index 'base' with indices from 'prev'
first_non_minus1 = np.argwhere(minimum_references != -1)[0][0]                  #           ^- index 3                              get the first valid index
filled[:first_non_minus1] = filled[first_non_minus1]                            #  9  9  9  9  9  9  9  9  9  9  9  9  7  7  7  7   set until valid index (3) the value in valid index (9)

ediff = np.ediff1d(filled)                                                      #  0  0  0  0  0  0  0  0  0  0  0 -2  0  0  0      check the places with discontinuity (reference change)
all_but_first = np.argwhere(ediff != 0).flatten() + 1                           # [11] + 1 = [12]                                   get the index of those discontinuity places (sum 1 since it's shifted to the left)
sections = np.hstack(([0], all_but_first))                                      # [0] + [12] = [0, 12]                              add the first section (starts at the beginning)

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





