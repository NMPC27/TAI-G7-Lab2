import re
from sys import stdin


def separate_sentences(s: str) -> str:
    return re.sub(r'\.[ ]?', '\n', s)

def no_references(s: str) -> str:
    return re.sub(r'\[\d+\]', '', s)

def no_blanklines(s: str) -> str:
    return re.sub(r'\n[\n]+', '\n', s)

def no_tabs_or_whitespace(s: str) -> str:
    return re.sub(r'\t| ([ ]+)', '', s)


if __name__ == '__main__':
    ipt = ''.join(stdin.readlines())
    
    print(no_references(ipt))
