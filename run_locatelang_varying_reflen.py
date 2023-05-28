import os
import subprocess
import numpy as np
import shutil
from typing import List

reference_portions = np.arange(0.1, 1.1, 0.1)   # dumb, ends up creating and using a 100% copy of the references at the end

references_folder_original = "example/reference"
references_folder_tmp = "example/reference_small"
file_target = "dog"

locatelang_command = ['python3', 'scripts/locatelang.py', '-t', 'example/target/' + file_target + ".txt", '-r', references_folder_tmp, '-p', '12']
cpm_args = ['--', '-p', 'c:10:3', '-t', 'c:0.01']
evaluate_command = ['python3', 'scripts/evaluate.py', 'example/target/' + file_target + ".txt"]

def run_and_write_results(locatelang_command: List[str], evaluate_command: List[str], f_to_write_to):
    locatelang_command_tmp = locatelang_command.copy()
    locatelang_command_tmp.extend(cpm_args)

    locate_lang_process = subprocess.run(locatelang_command_tmp, capture_output=True, input=b'y')
    evaluate_process = subprocess.run(evaluate_command, capture_output=True, input=locate_lang_process.stdout)

    if evaluate_process.returncode == 0 and locate_lang_process.returncode == 0:
        print("success:" + str(locatelang_command_tmp))
        f_to_write_to.write(evaluate_process.stdout.decode())

    else:
        print("failure: " + str(locatelang_command_tmp))
        print(locate_lang_process.stderr.decode())
        print(evaluate_process.stderr.decode())
    

with open('results/varying_reflen.txt', 'wt') as f:
    
    for reference_portion in reference_portions:
        
        print(f'Creating small references with size {reference_portion:%}...', end='')

        for reference in os.listdir(references_folder_original):
            reference_original_path = os.path.join(references_folder_original, reference)
            reference_tmp_path = os.path.join(references_folder_tmp, reference)

            with open(reference_original_path, 'rt') as fr:
                reference_content = fr.read()
            
            number_of_characters_to_use = int(reference_portion * len(reference_content))
            with open(reference_tmp_path, 'wt') as fr:
                fr.write(reference_content[:number_of_characters_to_use])

        print(' done!')

        try:
            shutil.rmtree('scripts/input/' + file_target)
        except:
            pass
        run_and_write_results(locatelang_command, evaluate_command, f)