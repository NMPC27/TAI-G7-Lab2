import os
import argparse
import asyncio
import matplotlib.pyplot as plt
import numpy as np
import numpy.typing as npt
from typing import Dict, List, Tuple, Any
from dataclasses import dataclass
from color_code import ColorCode


CACHED_INFORMATION_BIN_FORMAT = '{reference}.bin'


@dataclass
class TargetCacheInfo:
    """Information regarding the cached information bins, such as target file path and 'lang' arguments.
    Useful to know whether the cached bins can be used or not for the model arguments that were passed to the script."""

    target: str
    references_folder: str
    model_args: Dict[str, Any]

    def serialize(self) -> str:
        return self.target + '\n' + self.references_folder + '\n' + \
            '\n'.join(f"{k}\t{v}" for k, v in self.model_args.items())

    @classmethod
    def deserialize(cls, s: str) -> 'TargetCacheInfo':
        lines = s.splitlines(keepends=False)
        target = lines[0]
        references_folder = lines[1]

        split_tabs = lambda s: s.split('\t')
        model_args = dict(map(split_tabs, lines[2:]))
        
        return TargetCacheInfo(target, references_folder, model_args)


def low_pass_filter(signal: npt.ArrayLike, frequency_dropoff: float = 1e2) -> np.ndarray:
    """Apply a low pass filter over a signal.
    Downscales high frequencies exponentially on the frequency domain (FFT) and then rebuilds the signal (IFFT).
    `frequency_dropoff` controls how strong the exponential downscaling is (high values produce stronger filtering on lower frequencies).
    """
    
    out = np.fft.fft(signal)
    freq = np.fft.fftfreq(signal.size)
    return np.fft.ifft(out * np.e**-np.abs(frequency_dropoff * freq)).real


def spans_of_minimum_values(data: npt.ArrayLike, minimum_threshold: float, static_threshold: float, fill_unknown: bool = False) -> Tuple[np.ndarray, np.ndarray]:
    """From a matrix representing N parallel streams of the same size, identify which of the streams provided the minimum value and at what sections.
    
    Only minimum values that meet the following criteria are considered:
    - the absolute value is not larger than `(minimum_threshold * 100) %` of the second minimum, with `0 < minimum_threshold <= 1`
    - the absolute value is not larger than `static_threshold`

    Parameters
    ----------
    data : ArrayLike
        Matrix of size NxM containing the N streams each with M data points.
    minimum_threshold : float
        Value between 0 exclusive and 1 inclusive, indicating the maximum value that the minimums to be considered should have as a fraction of the second minimum.
    static_threshold : float
        Static threshold in bits below which to consider minimum values.
    fill_unknown : bool
        Whether to fill all unknown segments with one known minimum stream.
    
        If no minimum values are under the thresholds at a certain index, then the index is assigned to the last minimum stream up to this point.
        If no minimum values are under the thresholds at the beginning, then the section is assigned to the very first minimum stream.

    Returns
    -------
    sections : ndarray
        Array with the starting index of each identified section of minimum values
    stream_ids : ndarray
        Array with the stream identification (row index) of each identified section of minimum values
    """

    minimum_references = np.argmin(data, axis=0)
    
    minimum_2_references = np.sort(data, axis=0)[:2, :]
    minimums_to_consider = np.logical_and(((minimum_2_references[0, :] / minimum_2_references[1, :]) < minimum_threshold ), minimum_2_references[0, :] < static_threshold)

    minimum_references[~minimums_to_consider] = -1                                          # -1 -1 -1  9  9  9  9 -1 -1 -1 -1 -1  7  7 -1 -1   base

    if fill_unknown:
        # forward filling
        prev = np.arange(minimum_references.size)                                           #  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15   generate indices
        prev[~minimums_to_consider] = -1                                                    # -1 -1 -1  3  4  5  6 -1 -1 -1 -1 -1 12 13 -1 -1   set invalid indices to small number
        prev = np.maximum.accumulate(prev)                                                  # -1 -1 -1  3  4  5  6  6  6  6  6  6 12 13 13 13   accumulate maximum from left to right
        minimum_references = minimum_references[prev]                                       #  ?  ?  ?  9  9  9  9  9  9  9  9  9  7  7  7  7   index 'base' with indices from 'prev'

        # backward filling (first section, if unknown)
        all_non_minus1_found = np.argwhere(minimum_references != -1).flatten()
        if all_non_minus1_found.size != 0:                                                  #  ?  ?  ?  9  9  9  9  9  9  9  9  9  7  7  7  7
            first_non_minus1 = all_non_minus1_found[0]                                      #           ^- index 3                              get the first valid index
            minimum_references[:first_non_minus1] = minimum_references[first_non_minus1]    #  9  9  9  9  9  9  9  9  9  9  9  9  7  7  7  7   set until valid index (3) the value in valid index (9)

    ediff = np.ediff1d(minimum_references)                                                  #  0  0  0  0  0  0  0  0  0  0  0 -2  0  0  0      check the places with discontinuity (reference change)
    all_but_first = np.argwhere(ediff != 0).flatten() + 1                                   # [11] + 1 = [12]                                   get the index of those discontinuity places (sum 1 since it's shifted to the left)
    sections = np.hstack(([0], all_but_first))                                              # [0] + [12] = [0, 12]                              add the first section (starts at the beginning)

    return sections, minimum_references[sections]


def setup_target_bins_cache(target_path: str, references_folder: str, bins_folder: str, lang_args: List[str]) -> Tuple[str, str]:
    target_identifier, _ = os.path.splitext(os.path.basename(target_path))
    target_cache_path = os.path.join(bins_folder, target_identifier)
    target_cache_info_path = os.path.join(target_cache_path, '.info')
    
    lang_parser = argparse.ArgumentParser()
    lang_parser.add_argument('-k', type=int)
    lang_parser.add_argument('-a', type=float)
    lang_parser.add_argument('-p', type=str)
    lang_parser.add_argument('-r', type=str)
    lang_parser.add_argument('-t', type=str)
    lang_args_dict = vars(lang_parser.parse_args(lang_args))

    # Target and model info that the current script execution demands
    requested_target_info = TargetCacheInfo(target_path, references_folder,  {k:v for k, v in lang_args_dict.items() if v is not None})

    invalid_cache = False

    # If the cache is not setup, then create the cache folder and info
    if not os.path.isdir(target_cache_path):
        os.makedirs(target_cache_path)
        with open(target_cache_info_path, 'wt') as f:
            f.write(requested_target_info.serialize())
    
    # If the cache is already setup, check if it still refers to the same target and was calculated with the same model parameters
    else:
        with open(target_cache_info_path, 'rt') as f:
            cached_target_info = TargetCacheInfo.deserialize(f.read())
        
        if cached_target_info.target != target_path:
            raise ValueError(f'the target file has the same name as a cached entry, but they are from different paths (path already in cache: {target_cache_info_path})!')

        if cached_target_info.references_folder != references_folder:
            raise ValueError(f'the specified references folder is not the same as the references folder used for calculating the cached bins (\'{references_folder}\' != \'{cached_target_info.references_folder}\')!')

        specified_keys = cached_target_info.model_args.keys() | requested_target_info.model_args.keys()
        non_matching_model_parameters = {}
        for key in specified_keys:
            if key not in cached_target_info.model_args:
                non_matching_model_parameters[key] = (None, requested_target_info.model_args[key])
            elif key not in requested_target_info.model_args:
                non_matching_model_parameters[key] = (cached_target_info.model_args[key], None)
            elif cached_target_info.model_args[key] != requested_target_info.model_args[key]:
                non_matching_model_parameters[key] = (cached_target_info.model_args[key], requested_target_info.model_args[key])
        
        if non_matching_model_parameters:
            print(f'Warning: the cached entries for target {target_identifier} were calculated using different model parameters:')
            table_line_fmt = '{}\t{:<10}\t{:<10}'
            print(table_line_fmt.format('', 'cached', 'requested'))
            default_val_str = lambda v: '<default>' if v is None else v
            for key, (cached, requested) in non_matching_model_parameters.items():
                print(table_line_fmt.format(key, default_val_str(cached), default_val_str(requested)))
            answer = input('Do you want to recalculate and overwrite the cached entries? [y/N]: ')

            if answer.lower() != 'y':
                print('Overwrite not allowed, quitting...')
                exit(0)

            else:
                with open(target_cache_info_path, 'wt') as f:
                    f.write(requested_target_info.serialize())
                invalid_cache = True
    
    return target_identifier, target_cache_path, invalid_cache


AVAILABLE_COLORS = [color for color in ColorCode if color != ColorCode.END]

def print_labeled_target_terminal(minimum_references: npt.ArrayLike, minimum_references_locations: npt.ArrayLike, target_path: str, data_to_filename: Dict[str, str]):
    
    print('\nLabeled output:')

    all_references = np.unique(minimum_references)
    if len(all_references) > len(AVAILABLE_COLORS):
        print(f'Warning: the number of references is greater than the number of avaiblable output colors! (max. {len(AVAILABLE_COLORS)})')

    color_mapping = {
        reference: AVAILABLE_COLORS[i % len(AVAILABLE_COLORS)]
        for i, reference in enumerate(all_references)
    }

    with open(target_path, 'rt') as target:
        target_str = target.read()

    for reference, reference_location, reference_location_next in zip(minimum_references, minimum_references_locations, minimum_references_locations[1:]):
        print(color_mapping[reference].foreground() + target_str[reference_location:reference_location_next] + ColorCode.END.value, end='')
    print(color_mapping[minimum_references[-1]].foreground() + target_str[minimum_references_locations[-1]:] + ColorCode.END.value)

    print('Legend:')
    for reference, color in color_mapping.items():
        print(color.background() + data_to_filename[reference] + ColorCode.END.value, end=' ')
    print()


async def calculate_references_multiprocess(target_path: str, lang_args: List[str], references_to_calculate: List[str], references_folder: str, cache_folder: str, max_parallel_processes: int = 1, quit_at_error: bool = False):
    lang_path = os.path.join('bin', 'lang')
    print(f'Information bins are missing, will calculate using copy model in \'{lang_path}\'')

    lang_sub_processes = []
    references_to_calculate_remaining = list(references_to_calculate)
    n_references_to_calculate = len(references_to_calculate)
    progress = 0

    def print_progress(message: str):
        print(f'[{progress / n_references_to_calculate:.2%}] {message:100}', end='\r')

    async def launch_subprocess_lang(process_task_registry: List[asyncio.Task]):
        reference = references_to_calculate_remaining.pop()
        reference_path = os.path.join(references_folder, reference)
        reference_cached_result_path = os.path.join(cache_folder, CACHED_INFORMATION_BIN_FORMAT.format(reference=reference))

        process = await asyncio.create_subprocess_exec(lang_path, *lang_args, '-v', 'm:' + reference_cached_result_path, reference_path, target_path,
                                                       stdout=asyncio.subprocess.DEVNULL, stderr=asyncio.subprocess.PIPE, stdin=asyncio.subprocess.PIPE)
        process_task_registry.append(asyncio.create_task(process.communicate(input=b'y')))     # send the 'y' to standard input to accept overwriting the machine output file if needed
        print_progress(f'Launched subprocess running reference {reference}...')

    # Launch the first max_process processes, to kickstart the next loop
    print_progress('Starting to analyze non-cached references...')
    for _ in range(max_parallel_processes):
        if len(references_to_calculate_remaining) > 0:
            await launch_subprocess_lang(lang_sub_processes)
    
    failed_by_error = False
    while len(lang_sub_processes) > 0:
        next_subprocesses = []
        for process in asyncio.as_completed(lang_sub_processes):
            _, stderr = await process
            
            if failed_by_error:
                continue

            if stderr:
                print('\n')
                print(f'WARNING: possible error occurred during execution of the process! Outputting the captured stderr:')
                print(ColorCode.RED.foreground(), stderr.decode(), ColorCode.END.value, sep='')
                if quit_at_error:
                    failed_by_error = True
                continue

            progress += 1
            print_progress('Information bin for a reference calculated and cached.')

            if len(references_to_calculate_remaining) > 0:
                await launch_subprocess_lang(next_subprocesses)
        
        lang_sub_processes = next_subprocesses
    
    if failed_by_error:
        print('Quitting due to possible error.')
        raise RuntimeError('at least one execution of \'lang\' was unsusccessful')


def main(
    target_path: str,
    lang_args: List[str],
    bins_folder: str,
    references_folder: str,
    minimum_threshold: float = 2,
    n_processes: int = 1,
    print_labeled_target: bool = False,
    fill_unknown: bool = False,
    static_threshold_modifier: float = None,
    low_pass_filter_dropoff: float = 1e2,
    quit_at_error: bool = False,
    plot: bool = False,
    save_result: str = None
):
    
    references = set(reference for reference in os.listdir(references_folder) if reference != '.empty')
    target_identifier, target_cache_path, invalid_cache = setup_target_bins_cache(target_path, references_folder, bins_folder, lang_args)

    # If the information streams haven't been calculated yet, do so now
    # Assume the CACHED_INFORMATION_BIN_FORMAT has a 0-character prefix and 4-character suffix
    cached_references = {cached_bin[:-4] for cached_bin in os.listdir(target_cache_path) if cached_bin != '.info'}
    not_cached_references = references if invalid_cache else (references - cached_references)
    n_not_cached_references = len(not_cached_references)
    if n_not_cached_references > 0:
        asyncio.run(calculate_references_multiprocess(
            target_path=target_path,
            lang_args=lang_args,
            references_to_calculate=not_cached_references,
            references_folder=references_folder,
            cache_folder=target_cache_path,
            max_parallel_processes=n_processes,
            quit_at_error=quit_at_error,
        ))

    else:
        print('No information bins were required to be computed, proceeding!')

    information_bins = [f for f in os.listdir(target_cache_path) if f != '.info']

    if len(information_bins) != len(references):
        raise RuntimeError("the number of calculated information '.bin' files is different from the number of references!")

    print('Loading cached information bins', end='', flush=True)

    information_stream_row = np.fromfile(os.path.join(target_cache_path, information_bins[0]), np.float64)
    information_streams = np.zeros((len(information_bins), information_stream_row.size))
    information_streams[0, :] = low_pass_filter(information_stream_row, low_pass_filter_dropoff)
    print('.', end='', flush=True)

    data_to_filename = {0: information_bins[0][:-4], -1: '<unknown>'}

    for i, information_bin in enumerate(information_bins[1:]):
        data_to_filename[i+1] = information_bin[:-4]
        information_stream_row = np.fromfile(os.path.join(target_cache_path, information_bin), np.float64)
        information_streams[i+1, :] = low_pass_filter(information_stream_row, low_pass_filter_dropoff)
        print('.', end='', flush=True)
    
    # Define a threshold using the target alphabet size
    def get_alphabet_size(filename):
        with open(filename, 'rt') as file:
            content = file.read()

        unique_letters = set(content)
        num_letters = len(unique_letters)

        return num_letters

    # Define the static threshold in terms of the cardinality of the target alphabet
    target_alphabet_size = get_alphabet_size(target_path)
    static_threshold = np.log2(target_alphabet_size) / static_threshold_modifier

    print(' done!')

    print('Detecting language spans...', end='', flush=True)
    minimum_references_locations, minimum_references = spans_of_minimum_values(information_streams, minimum_threshold, static_threshold, fill_unknown)
    print(' done!')

    print('Results:')
    print('sections=', list(minimum_references_locations), sep='')
    print('languages=', [data_to_filename[reference_i] for reference_i in minimum_references], sep='')

    if print_labeled_target:
        print_labeled_target_terminal(minimum_references, minimum_references_locations, target_path, data_to_filename)

    if plot:
        colors = [
            '#FF0000', '#00FF00', '#0000FF', '#FFFF00', '#00FFFF', '#FF00FF',
            '#800000', '#008000', '#000080', '#808000', '#008080', '#800080',
            '#FFA500', '#A52A2A', '#800080', '#FFC0CB', '#000000', '#FF69B4',
            '#7CFC00', '#8A2BE2', '#FF4500', '#00FF7F', '#1E90FF'
        ]
            
        # Information over time
        plt.figure(figsize=(15, 8))
        for i in range(len(information_bins)):
            plt.plot(information_streams[i, :], color = colors[i % len(colors)], label=data_to_filename[i])
        plt.plot([static_threshold] * information_streams.shape[1], label='<static threshold>')
        plt.title('Information of each symbol in the target after training on each reference')
        plt.xlabel('Target position')
        plt.ylabel('Information (bits)')
        plt.legend(bbox_to_anchor=(1.04, 1), borderaxespad=0)
        plt.tight_layout()

        if save_result:
            plt.savefig(save_result + '.png')
        else:
            plt.show()



if __name__ == '__main__':

    default_str = ' (default: %(default)s)'
    
    parser = argparse.ArgumentParser(
        prog='locatelang',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='''Given a target text, determine which languages compose the text, segmenting it in different languages if many were identified. Take reference text of various languages into account.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: locatelang -t <TARGET> -- -r n''')
    parser.add_argument('-t', '--target', required=True, type=str, help='target file of which to identify language segments')
    parser.add_argument('-m', '--minimum-threshold', type=float, default=1, help='how similar is the most likely reference allowed to be to the second most likely reference' + default_str)
    parser.add_argument('-s', '--static-threshold', type=float, default=1.2, help='a static, global threshold of bits for all languages, only below which are reference languages considered. The higher the value the harsher the threshold, and should ideally be larger than 1' + default_str)
    parser.add_argument('-r', '--references-folder', type=str, default=os.path.join('example', 'reference'), help='location containing the language reference text' + default_str)
    parser.add_argument('-p', '--processes', type=int, default=1, help='maximum number of language analysis processes to run in parallel' + default_str)
    parser.add_argument('-f', '--frequency-filter', type=float, default=1e2, help='how much to downscale high frequencies of information')
    parser.add_argument('--bins-folder', type=str, default=os.path.join('scripts', 'input'), help='location on which the compressed information results of the target will be cached to' + default_str)
    parser.add_argument('--labeled-output', action='store_true', help='whether to print the target text labeled with colors for each detected language')
    parser.add_argument('--fill-unknown', action='store_true', help='whether to identify unknown language segments as a known language using forward filling (and backward filling for the first section if unknown)')
    parser.add_argument('--ignore-errors', action='store_true', help='dont\'t quit if runtime errors from \'lang\' are suspected' + default_str)
    parser.add_argument('--plot', action='store_true', help='whether to plot the filtered information graph')
    parser.add_argument('--save-result', type=str, default=None, help='save the plot to a file, instead of showing them' + default_str)
    parser.add_argument('lang_args', nargs='*', help='arguments to the \'lang\' program')

    args = parser.parse_args()


    main(
        target_path=args.target,
        lang_args=args.lang_args,
        minimum_threshold=args.minimum_threshold,
        references_folder=args.references_folder,
        bins_folder=args.bins_folder,
        n_processes=args.processes,
        print_labeled_target=args.labeled_output,
        fill_unknown=args.fill_unknown,
        static_threshold_modifier=args.static_threshold,
        low_pass_filter_dropoff=args.frequency_filter,
        quit_at_error=not args.ignore_errors,
        plot=args.plot,
        save_result=args.save_result
    )
