import subprocess
import shutil

lista = [
#    '-p f',
#    '-p u',
#    '-p c:??',
#    '-r m',
#    '-r c:100',
    '-r o',
    '-r n',
#    '-t n:0.5',
#    '-t f:9',
#    '-t c:0.01',

]

command = ['python3', 'scripts/locatelang.py', '-t', 'example/target/all_languages_random.txt', '-p', '6', '-f', '1000','--plot', '--save-result']

shutil.rmtree("scripts/input/all_languages_random")
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

    shutil.rmtree("scripts/input/all_languages_random")

