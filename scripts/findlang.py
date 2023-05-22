import os
import argparse
import asyncio
from typing import List, Dict


async def calculate_total_information_multiprocess(target_path: str, lang_args: List[str], references: List[str], references_folder: str, max_parallel_processes: int = 1, quit_at_error: bool = False) -> Dict[str, float]:
    lang_path = os.path.join('bin', 'lang')
    print(f'Using copy model in \'{lang_path}\'')

    lang_sub_processes = []
    references_remaining = list(references)
    n_references_to_calculate = len(references)
    progress = 0
    def print_progress(message: str):
        print(f'[{progress / n_references_to_calculate:.2%}] {message:100}', end='\r')

    async def include_reference_name(coro, reference: str):
        res = await coro
        return res, reference

    async def launch_subprocess_lang(process_task_registry: List[asyncio.Task]):
        reference = references_remaining.pop()
        reference_path = os.path.join(references_folder, reference)

        process = await asyncio.create_subprocess_exec(lang_path, *lang_args, '-v', 'o', reference_path, target_path, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
        process_task_registry.append(
            asyncio.create_task(
                include_reference_name(
                    process.communicate(), reference)))
        
        print_progress(f'Launched subprocess running reference {reference}...')

    informations = {reference:None for reference in references}

    # Launch the first max_process processes, to kickstart the next loop
    print_progress('Starting to analyze references...')
    for _ in range(max_parallel_processes):
        if len(references_remaining) > 0:
            await launch_subprocess_lang(lang_sub_processes)
    
    failed_by_error = False
    while len(lang_sub_processes) > 0:
        next_subprocesses = []
        for process in asyncio.as_completed(lang_sub_processes):
            (stdout, stderr), reference = await process

            if failed_by_error:
                continue

            if stderr:
                print('\n')
                print(f'WARNING: possible error occurred during execution of the process! Outputting the captured stderr:')
                print('\033[0;31m', stderr.decode(), '\033[0m', sep='')
                if quit_at_error:
                    print('Quitting due to possible error.')
                    failed_by_error = True

            informations[reference] = float(stdout)

            progress += 1
            print_progress(f'Information total for reference \'{reference}\' calculated.')

            if len(references_remaining) > 0:
                await launch_subprocess_lang(next_subprocesses)
        
        lang_sub_processes = next_subprocesses

    return informations


def main(
    target_path: str,
    lang_args: List[str],
    references_folder: str,
    n_processes: int = 1,
    quit_at_error: bool = False,
):

    references = [reference for reference in os.listdir(references_folder) if reference != '.empty']
    
    informations = asyncio.run(calculate_total_information_multiprocess(target_path, lang_args, references, references_folder, n_processes, quit_at_error))

    detected_language, detected_language_information = min(informations.items(), key=lambda t: t[1])
    print(f'Detected language of reference \'{detected_language}\', compressing the target down to {detected_language_information} bits.')


if __name__ == '__main__':

    default_str = ' (default: %(default)s)'

    parser = argparse.ArgumentParser(
        prog='findlang',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='''Run the 'lang' program with all reference texts against a given target text and output the language with the lowest reported entropy.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: findLang -t <TARGET> -- -r n''')
    parser.add_argument('-t', '--target', required=True, help='path to the target text whose language will be estimated')
    parser.add_argument('-r', '--references-folder', type=str, default=os.path.join('example', 'reference'), help='location containing the language reference text' + default_str)
    parser.add_argument('-p', '--processes', type=int, default=1, help='maximum number of language analysis processes to run in parallel' + default_str)
    parser.add_argument('-q', '--quit-at-error', action='store_true', help='whether the script should quit as soon as an error from \'lang\' is suspected' + default_str)
    parser.add_argument('lang_args', nargs='*', help='arguments to the \'lang\' program')

    args = parser.parse_args()

    main(args.target, args.lang_args, args.references_folder, args.processes, args.quit_at_error)