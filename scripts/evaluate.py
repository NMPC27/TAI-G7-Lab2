import argparse
from typing import List
import numpy as np
import json

from sys import stdin


def calculate_accuracy(result_sections: List[int], result_languages: List[str], ideal_sections: List[int], ideal_languages: List[str], total_symbols: int) -> float:
    if len(result_sections) != len(ideal_sections):
        raise ValueError(f'The result and ideal section arrays should have the same shape! ({len(result_sections)} != {len(ideal_sections)})')

    if len(result_languages) != len(ideal_languages):
        raise ValueError(f'The result and ideal language arrays should have the same size! ({len(result_languages)} != {len(ideal_languages)})')

    total_languages = set(result_languages) | set(ideal_languages)
    language_id_mapping = {language: i for i, language in enumerate(total_languages)}

    predictions = np.arange(total_symbols)
    actual = np.arange(total_symbols)

    for section_begin, section_end, language in zip(result_sections, result_sections[1:] + [-1], result_languages):
        predictions[section_begin:section_end] = language_id_mapping[language]

    for section_begin, section_end, language in zip(ideal_sections, ideal_sections[1:] + [-1], ideal_languages):
        actual[section_begin:section_end] = language_id_mapping[language]

    return np.sum(predictions == actual) / total_symbols


def parse_list_str(line: str) -> np.ndarray:
    array_str = ''.join(line.strip().split('=')[1:]).replace('\'', '"')
    return json.loads(array_str)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        prog='evaluate',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='''Calculate the language classification given a language segmentation result and its ideal segmentation.
The ideal and result segmentations should be 2 attributes in the following format, specifying at which index in the target file a section starts and which language it is from:

sections=[<target_idx_0>, <target_idx_1>, <target_idx_2>, ..., <target_idx_n>]
languages=['<reference_name_0>', '<reference_name_1>', '<reference_name_2>', ..., '<reference_name_n>']

The classification result is meant to be provided into the program's standard input as lines.

Example: echo 'y' | python3 scripts/locatelang.py ... | evaluate <TARGET>

The program outputs the classification accuracy.''')

    parser.add_argument('target', type=str, help='path to the target file where the text resides (should have a <target>.eval file at the same path as well)')

    args = parser.parse_args()
    target_path = args.target
    target_eval_path = target_path + '.eval'

    ideal_languages = None
    ideal_sections = None

    with open(target_eval_path, 'rt') as target:
        for line in target:
            if line.startswith('sections='):
                ideal_sections = parse_list_str(line)
            
            if line.startswith('languages='):
                ideal_languages = parse_list_str(line)

    result_languages = None
    result_sections = None

    for line in stdin:

        if line.startswith('sections='):
            array_str = ''.join(line.split('=')[1:])
            result_sections = parse_list_str(line)

        if line.startswith('languages='):
            array_str = ''.join(line.split('=')[1:])
            result_languages = parse_list_str(line)
    
    with open(target_path, 'rt') as f:
        total_symbols = len(f.read())

    print(f'Classification accuracy: {calculate_accuracy(result_sections, result_languages, ideal_sections, ideal_languages, total_symbols):%}')

