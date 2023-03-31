# TAI-G7-Lab2

## Repository structure

- `src`: source code of both `cpm` and `cpm_gen`
- `bin`: compiled binaries of `cpm` and `cpm_gen`
- `example`: test files to run the `cpm` and `cpm_gen` with
- `report`: report detailing the developed model and the decisions taken during development
- `scripts`: helper data analysis Python3 scripts

## Requirements
CMake >= 3.10

## Building

In order to build the project, first create a build directory and generate a CMake native build environment. If using Linux, run:

```
mkdir build
cd build
cmake ..
```

After that, build the binaries, within the `build` directory:

```
cmake --build .
```

The generated binaries, both cpm and cpm_gen will be present in the `bin` folder.

## Usage

### `cpm`

To use the copy model, run:

```
./bin/cpm [OPTIONS] <file>
```

Or run the bash script run.sh, which also already compiles the program if the `build` folder exists:

```
./run.sh [OPTIONS] <file>
```

```
Options:
        -h              Show this help message
        -v V            Additional output (verbose modes output the probability distribution at each encoding step):
                                h - Human-readable verbose output, color-coded depending on whether a hit/miss/guess occurred
                                m - Machine-readable verbose output, without color-coding and minimal flair (CSV format with header)
                                p - Print the progress of processing the sequence
        -k K            Size of the sliding window (default: 12)
        -a A            Smoothing parameter alpha for the prediction probability (default: 1.0)
        -p P            Probability distribution of the characters other than the one being predicted (default: f):
                                u - uniform distribution
                                f - distribution based on the symbols' relative frequencies
        -r R            Copy pointer reposition (default: m):
                                o - oldest
                                n - newer
                                m - most common prediction among all pointers
        -t T            Threshold for copy pointer switch (default: f:6):
                                n:X - static probability below X
                                f:X - number of successive fails above X
                                c:X - absolute value of the negative derivative of the prediction probability above X
```

### `cpm_gen`

To use the copy model, run:

```
./bin/cpm_gen -s 'string inicial' -f train_file.txt -n num_caracteres_a_gerar [OPTIONS]
```
```
Options:
  -h  help
  -t  (allow training himself)
  -l  (allow only lower case letters - better for smaller input texts)
```

## Example runs

### `cpm`

Best run
```
./run.sh -k 12 -a 1 -r m -t f:6 ./example/chry.txt

Total amount of information: 4.2215e+07 bits | media: 1.8623

Time Taken: 2m30,314s
```

### `cpm_gen`

Exam
```
./bin/cpm_gen -s 'As the' -f ./example/othello.txt -n 1000 
```
