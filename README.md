# TAI-G7-Lab2

## Repository structure

- `src`: source code of `lang`
- `bin`: compiled binary for `lang`
- `example`: folder containing the reference and target files to run with `lang`, `findlang.py` and `locatelang.py`
- `report`: report detailing the developed `lang` program and analysis scripts, the decisions taken during development and results
- `scripts`: `findlang.py` and `locatelang.py` scripts. Also contains the `evaluate.py`, which takes the output from `locatelang.py` and calculates the classification accuracy.
- `scripts/helper`: helper scripts used for creating results and plotting, not relevant for non-developers.
- `results`: extensive results obtained from executing the programs, not relevant for non-developers. For readable results, refer to the report in the `report` folder

## Requirements

- CMake >= 3.10, C++ 17 standard compiler.
- Python >= 3.8.10, NumPy, Matplotlib.

## Building

In order to build the project, first create a build directory and generate a CMake native build environment. If using Linux, run:

```
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
```

For a debug build, run the following:

```
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

After that, build the binaries, within the `release`/`debug` directory:

```
cmake --build .
```

The generated binary `lang` will be present in the `bin` folder.

## Usage

### Lang

```
Usage: ./bin/lang [OPTIONS] reference target
Options:
        -h              Show this help message
        -v V            Additional output:
                                h - Human-readable probability distributions at each step, color-coded depending on whether a hit/miss/guess occurred
                                m:X - Output the symbol information at each step to the file X, as a sequence of 8-byte doubles. Also prints progress (default: lang.bin)
                                p - Print the progress of processing the sequence
                                o - Print only the total number of bits to standard output
        -k K            Size of the sliding window (default: 12)
        -a A            Smoothing parameter alpha for the prediction probability (default: 1.0)
        -p P            Probability distribution of the characters other than the one being predicted (default: f):
                                u - uniform distribution
                                f - distribution based on the symbols' relative frequencies
                                c:A:K - distribution based on a first-order finite context model, with alpha A and context size K (default A: 1.0, default K: 3)
        -r R            Copy pointer reposition (default: m):
                                o - oldest
                                n - newer
                                m - most common prediction among all pointers
                                c:X - most common prediction among all pointers, bounded by X entries
        -t T            Threshold for copy pointer switch (default: f:6):
                                n:X - static probability below X
                                f:X - number of successive fails above X
                                c:X - absolute value of the negative derivative of the prediction probability above X
```

### Find lang

```
findlang [-h] -t TARGET [-r REFERENCES_FOLDER] [-p PROCESSES] [--ignore-errors] [lang_args ...]

Run the 'lang' program with all reference texts against a given target text and output the language with the lowest reported entropy.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: findlang -t <TARGET> -- -r n

positional arguments:
  lang_args             arguments to the 'lang' program

options:
  -h, --help            show this help message and exit
  -t TARGET, --target TARGET
                        path to the target text whose language will be estimated
  -r REFERENCES_FOLDER, --references-folder REFERENCES_FOLDER
                        location containing the language reference text (default: example/reference)
  -p PROCESSES, --processes PROCESSES
                        maximum number of language analysis processes to run in parallel (default: 1)
  --ignore-errors       don't quit if runtime errors from 'lang' are suspected (default: False)
```

### Locate lang

```
locatelang [-h] -t TARGET [-m MINIMUM_THRESHOLD] [-s STATIC_THRESHOLD] [-r REFERENCES_FOLDER] [-p PROCESSES] [-f FREQUENCY_FILTER] [--bins-folder BINS_FOLDER] [--labeled-output]
                  [--fill-unknown] [--ignore-errors] [--plot] [--save-result SAVE_RESULT]
                  [lang_args ...]

Given a target text, determine which languages compose the text, segmenting it in different languages if many were identified. Take reference text of various languages into account.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: locatelang -t <TARGET> -- -r n

positional arguments:
  lang_args             arguments to the 'lang' program

options:
  -h, --help            show this help message and exit
  -t TARGET, --target TARGET
                        target file of which to identify language segments
  -m MINIMUM_THRESHOLD, --minimum-threshold MINIMUM_THRESHOLD
                        how similar is the most likely reference allowed to be to the second most likely reference (default: 1)
  -s STATIC_THRESHOLD, --static-threshold STATIC_THRESHOLD
                        a static, global threshold of bits for all languages, only below which are reference languages considered. The higher the value the harsher the threshold, and should ideally
                        be larger than 1 (default: 1.2)
  -r REFERENCES_FOLDER, --references-folder REFERENCES_FOLDER
                        location containing the language reference text (default: example/reference)
  -p PROCESSES, --processes PROCESSES
                        maximum number of language analysis processes to run in parallel (default: 1)
  -f FREQUENCY_FILTER, --frequency-filter FREQUENCY_FILTER
                        how much to downscale high frequencies of information
  --bins-folder BINS_FOLDER
                        location on which the compressed information results of the target will be cached to (default: scripts/input)
  --labeled-output      whether to print the target text labeled with colors for each detected language
  --fill-unknown        whether to identify unknown language segments as a known language using forward filling (and backward filling for the first section if unknown)
  --ignore-errors       dont't quit if runtime errors from 'lang' are suspected (default: False)
  --plot                whether to plot the filtered information graph
  --save-result SAVE_RESULT
                        save the plot to a file, instead of showing them (default: None)
```

### Verbose mode: Machine (`-v m`)

The output of this mode will be an array of consecutive 8-byte floating point numbers.
Below is an example of how these may be written in Python using NumPy:

```python
>>> import numpy as np
>>> information_at_each_step = np.fromfile('lang.bin', np.float64)
array([9.77614296, 9.7864965 , 2.66104321, ..., 0.05050169, 0.07076569, 0.05976416])
```

## Example runs

### Lang

```
./bin/lang -vp example/reference/en_GB.English-latn-EP7.utf8 example/target/dog.txt
```

### Find lang

```
python3 scripts/findlang.py -t example/target/dog.txt -p 4
```

### Locate lang

```
python3 scripts/locatelang.py -t example/target/all_languages.txt -p 4 --plot -- -p  c:10:3
```

