import subprocess
import shutil
import numpy as np

lista = [
#    '-p f',
#    '-p u',
#    '-r m',
#    '-r c:100',
#    '-r o',
#    '-r n',
#    '-t n:0.5',
#    '-t f:9',
#    '-t c:0.01',
#    '-p c:1:3',
#    '-p c:10:3',
#    '-p c:100:3',
#    '-p c:1:3',
#    '-p c:1:6',
#    '-p c:1:9',
]

lista_ll = [
    ('-m {}', np.arange(1.0, 0.79, -0.01)),
    ('-s {}', np.arange(1.0, 2.05, 0.05)),
    ('-f {}', np.arange(100, 2100, 100)),
]

file_target = "all_languages"

command = ['python3', 'scripts/locatelang.py', '-t', 'example/target/'+file_target+".txt", '-p', '6', '-f', '1000','--plot', '--save-result']

# try:
#     shutil.rmtree("scripts/input/"+file_target)
# except:
#     pass

for arg in lista:
    out=arg.replace(' ', '_')
    print("Running: " + arg)

    tmp = command.copy()
    tmp.append("results/"+out)
    tmp.append('--')

    for i in arg.split(' '):
        tmp.append(i)

    result = subprocess.run(tmp, capture_output=True, text=True, input='y')

    if result.returncode == 0:
        print("success:" + str(tmp))
        with open('results/'+out+'.txt', 'w') as file:
            file.write(result.stdout)
    else:
        print("failure: " + str(tmp))
        print(result.stderr)
        print(result.stdout)

    # shutil.rmtree("scripts/input/"+file_target)


command = ['python3', 'scripts/locatelang.py', '-t', 'example/target/' + file_target + ".txt", '-p', '12']
cpm_args = ['--', '-p', 'c:10:3', '-t', 'c:0.01']
evaluate_command = ['python3', 'scripts/evaluate.py', 'example/target/' + file_target + ".txt"]

for arg_fmt, values in lista_ll:
    out = arg_fmt.split(' ')[0]
    with open('results/ll' + out + '.txt', 'wt') as f:
    
        for value in values:
            print("Running: " + arg_fmt.format(value))

            tmp = command.copy()
            tmp.append(arg_fmt.format(value))
            tmp.extend(cpm_args)

            locate_lang_process = subprocess.run(tmp, capture_output=True, input=b'y')
            evaluate_process = subprocess.run(evaluate_command, capture_output=True, input=locate_lang_process.stdout)

            if evaluate_process.returncode == 0 and locate_lang_process.returncode == 0:
                print("success:" + str(tmp))
                f.write(evaluate_process.stdout.decode())

            else:
                print("failure: " + str(tmp))
                print(locate_lang_process.stderr.decode())
                print(evaluate_process.stderr.decode())
