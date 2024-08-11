import glob
from itertools import accumulate

import click
import matplotlib.pyplot as plt


def parse_eval_file(path, inf_val=600.0):
    """Parse chutechess output and return elo difference.

    Parameters
    ----------
    inf_val:
        Elo difference to use when side scored 100%.
    """
    with open(path, 'r') as f:
        for line in f:
            if line.startswith('Elo'):
                val = float(line.split()[2])
                if val == float("inf"):
                    return inf_val
                return val
    raise ValueError


@click.command
@click.argument('pattern')
@click.option('-g', '--games-per', default=3200, help='selfplay games per iteration')
@click.option('-d', '--dark', is_flag=True, help='dark background')
@click.option('-s', '--save', is_flag=True)
@click.option('--cumulative/--no-cumulative', is_flag=True, default=True)
def main(pattern, games_per, dark, save, cumulative):
    """Plot ELO vs games by parsing cutechess-cli eval output.

    PATTERN is a shell glob pattern used to identify a set of evaluation output files produced by cutechess-cli.

    """
    paths = glob.glob(pattern)
    paths = sorted(paths, key=lambda x: int(x.split('.')[-2]))
    # print(paths)
    elo_diffs = [parse_eval_file(p) for p in paths]
    print(elo_diffs)
    if cumulative:
        elos = list(accumulate(elo_diffs))
        elos.insert(0, 0)
        xvals = list(range(0, games_per * len(elo_diffs) + 1, games_per))
    else:
        elos = elo_diffs
        xvals = list(range(games_per, games_per * len(elos) + 1, games_per))

    if dark:
        plt.style.use('dark_background')

    f, ax = plt.subplots()
    ax.plot(xvals, elos, '-o')
    ax.set_xlabel('Selfplay games')
    ax.set_ylabel('Relative ELO')
    ax.grid()

    if save:
        f.savefig('elo.png')

    plt.show()


if __name__ == '__main__':
    main()
