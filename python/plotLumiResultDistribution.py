import argparse
import json
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(
    description='', formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('lumifit_results', metavar='', type=str, nargs=1,
                    help='Path to JSON LumiFit results file.')

args = parser.parse_args()

with open(args.lumifit_results[0], 'r') as infile:
    data = json.load(infile)
    
    values = data['relative_deviation_in_percent']
    values = [float(x) for x in values]
    a = plt.hist(values, 20)
    plt.show()
