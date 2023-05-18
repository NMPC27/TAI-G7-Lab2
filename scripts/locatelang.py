import os
import argparse
import subprocess
import asyncio
import numpy as np
import numpy.typing as npt
from typing import List, Tuple


CACHED_INFORMATION_BIN_FORMAT = 'lang_{reference}.bin'


def low_pass_filter(signal: npt.ArrayLike, frequency_dropoff: float = 1e4) -> np.ndarray:
    """Apply a low pass filter over a signal.
    Downscales high frequencies exponentially on the frequency domain (FFT) and then rebuilds the signal (IFFT).
    `frequency_dropoff` controls how strong the exponential downscaling is (high values produce stronger filtering on lower frequencies).
    """
    
    out = np.fft.fft(signal)
    freq = np.fft.fftfreq(signal.size)
    return np.fft.ifft(out * np.e**-np.abs(frequency_dropoff * freq)).real


def spans_of_minimum_values(data: npt.ArrayLike, minimum_threshold: float) -> Tuple[np.ndarray, np.ndarray]:
    """From a matrix representing N parallel streams of the same size, identify which of the streams provided the minimum value and at what sections.
    
    Only minimum values under a given threshold are considered.

    Parameters
    ----------
    data : ArrayLike
        Matrix of size NxM containing the N streams as the rows.
    global_threshold : float
        Threshold below which to consider minimum values.

        If no minimum values are under the threshold at a certain index, then the index is assigned to the last minimum stream up to this point.
        If no minimum values are under the threshold at the beginning, then the section is assigned to the very first minimum stream.

    Returns
    -------
    sections : ndarray
        Array with the starting index of each identified section of minimum values
    stream_ids : ndarray
        Array with the stream identification (row index) of each identified section of minimum values
    """

    minimum_references = np.argmin(data, axis=0)
    minimums_to_consider = np.min(data, axis=0) < minimum_threshold

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

    return sections, filled[sections]


def setup_target_bins_cache(target_path: str, bins_folder: str) -> Tuple[str, str]:
    target_identifier, _ = os.path.splitext(os.path.basename(target_path))
    target_cache_path = os.path.join(bins_folder, target_identifier)
    target_cache_info_path = os.path.join(target_cache_path, '.info')
    
    # If the cache is not setup, then create the cache folder and info
    if not os.path.isdir(target_cache_path):
        os.makedirs(target_cache_path)
        with open(target_cache_info_path, 'wt') as f:
            f.write(target_path)
    
    # If the cache is already setup, check if it still refers to the same target
    else:
        with open(target_cache_info_path, 'rt') as f:
            if f.readline() != target_path:
                raise ValueError(f'the target file has the same name as a cached entry, but they are from different paths (path already in cache: {target_cache_info_path})!')
    
    return target_identifier, target_cache_path


async def calculate_references_multiprocess(target_path: str, lang_args: List[str], references_to_calculate: List[str], references_folder: str, cache_folder: str, max_parallel_processes: int = 1):
    lang_path = os.path.join('bin', 'lang')
    print(f'Information bins are missing, will calculate using copy model in \'{lang_path}\'')

    lang_sub_processes = []
    references_to_calculate_remaining = list(references_to_calculate)
    n_references_to_calculate = len(references_to_calculate)
    progress = 0

    def print_progress(message: str):
        print(f'[{progress / n_references_to_calculate:.2%}] {message:100}', end='\r')

    print_progress('Starting to analyze non-cached references...')
    async def launch_subprocess_lang(process_task_registry: List[asyncio.Task]):
        reference = references_to_calculate_remaining.pop()
        reference_path = os.path.join(references_folder, reference)
        reference_cached_result_path = os.path.join(cache_folder, CACHED_INFORMATION_BIN_FORMAT.format(reference=reference))

        process = await asyncio.create_subprocess_exec(lang_path, *lang_args, '-v', 'm:' + reference_cached_result_path, reference_path, target_path, stdout=asyncio.subprocess.DEVNULL)
        process_task_registry.append(asyncio.create_task(process.wait()))
        print_progress(f'Launched subprocess running reference {reference}...')

    # Launch the first max_process processes, to kickstart the next loop
    for _ in range(max_parallel_processes):
        if len(references_to_calculate_remaining) > 0:
            await launch_subprocess_lang(lang_sub_processes)
        
    while len(lang_sub_processes) > 0:
        next_subprocesses = []
        for process in asyncio.as_completed(lang_sub_processes):
            await process
            
            progress += 1
            print_progress('Information bin for a reference calculated and cached.')

            if len(references_to_calculate_remaining) > 0:
                await launch_subprocess_lang(next_subprocesses)
        
        lang_sub_processes = next_subprocesses


def main(
    target_path: str,
    lang_args: List[str],
    bins_folder: str,
    references_folder: str,
    minimum_threshold: float = 2,
    n_processes: int = 1,
):

    references = set(os.listdir(references_folder))
    target_identifier, target_cache_path = setup_target_bins_cache(target_path, bins_folder)

    # If the information streams haven't been calculated yet, do so now
    # Assume the CACHED_INFORMATION_BIN_FORMAT has a 5-character prefix and 4-character suffix
    cached_references = {cached_bin[5:-4] for cached_bin in os.listdir(target_cache_path) if cached_bin != '.info'}
    not_cached_references = references - cached_references
    n_not_cached_references = len(not_cached_references)
    if n_not_cached_references > 0:
        asyncio.run(calculate_references_multiprocess(
            target_path=target_path,
            lang_args=lang_args,
            references_to_calculate=not_cached_references,
            references_folder=references_folder,
            cache_folder=target_cache_path,
            max_parallel_processes=n_processes,
        ))

    else:
        print('No information bins were required to be computed, proceeding!')

    information_bins = [f for f in os.listdir(target_cache_path) if f != '.info']

    assert len(information_bins) == len(references), "the number of calculated information '.bin' files is different from the number of references!"

    print('Loading cached information bins', end='', flush=True)

    information_stream_row = np.fromfile(os.path.join(target_cache_path, information_bins[0]), np.float64)
    information_streams = np.zeros((len(information_bins), information_stream_row.size))
    information_streams[0, :] = low_pass_filter(information_stream_row)
    print('.', end='', flush=True)

    data_to_filename = {'0': information_bins[0]}

    for i, information_bin in enumerate(information_bins[1:]):
        data_to_filename[str(i+1)] = information_bin
        information_stream_row = np.fromfile(os.path.join(target_cache_path, information_bin), np.float64)
        information_streams[i+1, :] = low_pass_filter(information_stream_row)
        print('.', end='', flush=True)
    
    print(' done!')

    print('Detecting language spans with method 1...', end='', flush=True)
    minimum_references_locations, minimum_references = spans_of_minimum_values(information_streams, minimum_threshold)
    print(' done!')

    print('Method 1 results:')
    print(minimum_references_locations)
    print([data_to_filename[str(reference_i)] for reference_i in minimum_references])

    print('Detecting language spans with method 2...', end='', flush=True)
    # Generate contiguous intervals
    intervals = []
    start = None

    minimum_references = np.argmin(information_streams, axis=0)
    minimums_to_consider = np.min(information_streams, axis=0) < 2
    minimum_references[~minimums_to_consider] = -1   

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

    print(' done!')

    # Print intervals
    print('Method 2 results:')
    for interval in intervals:
        print(interval)



if __name__ == '__main__':

    default_str = ' (default: %(default)s)'
    
    parser = argparse.ArgumentParser(
        prog='locatelang',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='''Given a target text, determine which languages compose the text, segmenting it in different languages if many were identified. Take reference text of various languages into account.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: findLang -t <TARGET> -- -r n''')
    parser.add_argument('-t', '--target', required=True, type=str, help='target file of which to identify language segments')
    parser.add_argument('-m', '--minimum-threshold', type=float, default=2, help='threshold of bits only below which is a portion of compressed text to be considered of a reference language' + default_str)
    parser.add_argument('-r', '--references-folder', type=str, default=os.path.join('example', 'reference'), help='location containing the language reference text' + default_str)
    parser.add_argument('-p', '--processes', type=int, default=1, help='maximum number of language analysis processes to run in parallel' + default_str)
    parser.add_argument('--bins-folder', type=str, default=os.path.join('scripts', 'input'), help='location on which the compressed information results of the target will be cached to' + default_str)
    parser.add_argument('lang_args', nargs='*', help='arguments to the \'lang\' program')

    args = parser.parse_args()

    main(
        target_path=args.target,
        lang_args=args.lang_args,
        minimum_threshold=args.minimum_threshold,
        references_folder=args.references_folder,
        bins_folder=args.bins_folder,
        n_processes=args.processes,
    )
