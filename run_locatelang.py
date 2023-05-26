import subprocess
import shutil

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
    '-p c:1:3',
    '-p c:1:6',
#    '-p c:1:9',
]

file_target="all_languages_random"

command = ['python3', 'scripts/locatelang.py', '-t', 'example/target/'+file_target+".txt", '-p', '6', '-f', '1000','--plot', '--save-result']

try:
    shutil.rmtree("scripts/input/"+file_target)
except:
    pass

for arg in lista:
    out=arg.replace(' ', '_')
    print("Running: " + arg)

    tmp = command.copy()
    tmp.append("results/"+out)
    tmp.append('--')

    for i in arg.split(' '):
        tmp.append(i)

    result = subprocess.run(tmp, capture_output=True, text=True)

    if result.returncode == 0:
        print("success:" + str(tmp))
        with open('results/'+out+'.txt', 'w') as file:
            file.write(result.stdout)
    else:
        print("failure: " + str(tmp))
        print(result.stderr)
        print(result.stdout)

    shutil.rmtree("scripts/input/"+file_target)

