import os
import argparse
import matplotlib.pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument('accuracies_folder')
args = parser.parse_args()

accuracies_folder = args.accuracies_folder

for parameter in ['f', 'm', 's']:
    target = os.path.basename(accuracies_folder)[3:]
    with open(os.path.join(accuracies_folder, f'parsed-ll-{parameter}.txt'), 'rt') as f:
        accuracies = [float(line) for line in f]

    if parameter == 'f':
        x = np.arange(0, 105, 5)
        xlabel = 'Frequency dropoff'
        parameter_str = 'frequency dropoff'
    elif parameter == 'm':
        x = np.arange(1.0, 0.79, -0.01)
        xlabel = 'Minimum threshold'
        parameter_str = 'minimum threshold'
    elif parameter == 's':
        x = np.arange(1.0, 2.05, 0.05)
        xlabel = 'Static threshold (denominator)'
        parameter_str = 'static threshold'
    else:
        exit(1)

    plt.plot(x, accuracies)
    plt.title(f'Classification accuracy w.r.t. {parameter_str} ({target})')
    plt.xlabel(xlabel)
    plt.ylabel('Accuracy (%)')
    plt.savefig(os.path.join(accuracies_folder, f'll-{parameter}.png'))
    plt.close()