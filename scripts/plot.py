import matplotlib.pyplot as plt
import numpy as np


information_at_each_step = np.fromfile('lang.bin', dtype=np.float64)
frequency_dropoff = 1e4

out = np.fft.fft(information_at_each_step)
freq = np.fft.fftfreq(information_at_each_step.size)

outifft = np.fft.ifft(out * np.e**-np.abs(frequency_dropoff * freq))

plt.plot(outifft.real)

plt.show()
#plt.savefig('test.png')