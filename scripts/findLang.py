import subprocess
import argparse
from os import listdir, getcwd
from os.path import join
from typing import List


REFERENCES_FOLDER = 'references'


def print_progress(progress: float, message: str):
    print(f'[{progress:.2%}] {message:100}', end='\r')


def main(target: str, land_args: List[str], quit_at_error: bool=False):

    project_path = getcwd()
    lang_path = join(project_path, 'bin', 'lang')
    print(f'Using copy model in \'{lang_path}\'')

    references_folder_path = join(project_path, 'example', 'reference')
    target_path = join(project_path, target)

    print_progress(0, 'Starting to analyze references...')
    reference_names = listdir(references_folder_path)
    total_references = len(reference_names)
    entropies = {reference_name:None for reference_name in reference_names}
    
    for progress, reference_name in enumerate(reference_names):
        if reference_name == '.empty':
            continue
        
        reference_path = join(references_folder_path, reference_name)
        
        print_progress(progress / total_references, f'Running reference {reference_name}...')
        completed_process = subprocess.run([lang_path, *land_args, '-v', 'o', reference_path, target_path], capture_output=True)
        print_progress((progress + 1) / total_references, f'Entropy for reference {reference_name} calculated.')

        if completed_process.stderr:
            print('\n')
            print(f'WARNING: possible error occurred during execution of the process! Outputting the captured stderr:')
            print('\033[0;31m', completed_process.stderr.decode(), '\033[0m', sep='')
            if quit_at_error:
                print('Quitting due to possible error.')
                exit(1)

        entropies[reference_name] = float(completed_process.stdout)

    print_progress(1, 'Finished processing.')
    print()

    detected_language, detected_language_entropy = min(entropies.items(), key=lambda t: t[1])
    print(f'Detected language of reference \'{detected_language}\' with entropy of {detected_language_entropy} bits.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='findLang',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='''Run the 'lang' program with all reference texts against a given target text and output the language with the lowest reported entropy.
Should run at the root of the project.
In order to pass the list of arguments 'land_args', put those arguments at the end with '--' before specifying them, in order to not process '-X' as arguments to this script.

Example: findLang -t <TARGET> -- -r n''')
    parser.add_argument('-t', '--target', required=True, help='path to the target text whose language will be estimated')
    parser.add_argument('-q', '--quit-at-error', action='store_true', help='whether the script should quit as soon as an error from \'lang\' is suspected (default: False)')
    parser.add_argument('lang_args', nargs='*', help='arguments to the \'lang\' program')

    args = parser.parse_args()

    main(args.target, args.lang_args, args.quit_at_error)