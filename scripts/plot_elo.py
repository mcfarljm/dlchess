import glob
from itertools import accumulate

import click
import matplotlib.pyplot as plt


def parse_eval_file(path):
    with open(path, 'r') as f:
        for line in f:
            if line.startswith('Elo'):
                return float(line.split()[2])
    raise ValueError


@click.command
@click.argument('pattern')
@click.option('-g', '--games-per', default=3200, help='selfplay games per iteration')
@click.option('-s', '--save', is_flag=True)
def main(pattern, games_per, save):
    """Plot ELO vs games by parsing cutechess-cli eval output.

    PATTERN is a shell glob pattern used to identify a set of evaluation output files produced by cutechess-cli.

    """
    paths = glob.glob(pattern)
    paths = sorted(paths, key=lambda x: int(x.split('_')[1].split('.')[1]))
    # print(paths)
    elo_diffs = [parse_eval_file(p) for p in paths]
    print(elo_diffs)
    elos = list(accumulate(elo_diffs))
    elos.insert(0, 0)

    xvals = list(range(0, games_per * len(elo_diffs) + 1, games_per))

    f, ax = plt.subplots()
    ax.plot(xvals, elos, '-o')
    ax.set_xlabel('Selfplay games')
    ax.set_ylabel('ELO')
    ax.grid()

    if save:
        f.savefig('elo.png')

    plt.show()


if __name__ == '__main__':
    main()
