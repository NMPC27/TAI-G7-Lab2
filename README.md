# TAI-G7-Lab2

## Repository structure

**TODO**

## Requirements
CMake >= 3.10, C++ 17 standard compiler.

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

**TODO**

### Verbose mode: Machine (`-v m`)

The output of this mode will be an array of consecutive 8-byte floating point numbers.
Below is an example of how these may be written in Python using NumPy:

```python
>>> import numpy as np
>>> information_at_each_step = np.fromfile('lang.bin', np.float64)
array([9.77614296, 9.7864965 , 2.66104321, ..., 0.05050169, 0.07076569, 0.05976416])
```

## Example runs

**TODO**

