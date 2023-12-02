import os
import time

import torch
from torch import nn
import click

device = 'cpu'


class ChessNet(nn.Module):
    def __init__(self, in_channels=21, grid_size=8):
        self.in_channels = in_channels
        self.grid_size = grid_size
        super().__init__()
        self.pb = nn.Sequential(
            nn.Conv2d(in_channels, 64, 3, padding='same', bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding='same', bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding='same', bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding='same', bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),
        )

        self.policy_stack = nn.Sequential(
            nn.Conv2d(64, 64, 3, padding='same', bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 73, 3, padding='same'),
        )

        self.value_stack = nn.Sequential(
            nn.Conv2d(64, 1, 1, bias=False),
            nn.BatchNorm2d(1),
            nn.ReLU(),

            nn.Flatten(),
            nn.Linear(grid_size**2, 256),
            nn.ReLU(),

            nn.Linear(256, 1),
            nn.Tanh(),
        )

    def forward(self, x):
         pb = self.pb(x)
         policy = self.policy_stack(pb)
         value = self.value_stack(pb)
         return policy, value


def count_parameters(model):
    return sum(p.numel() for p in model.parameters() if p.requires_grad) 


# with torch.no_grad():
#     def chunker(seq, size):
#         return (seq[pos:pos + size] for pos in range(0, len(seq), size))

#     print('default threads:', torch.get_num_threads())
#     # torch.set_num_threads(1)
#     grid_size = 8
#     encoder_channels = 21
#     model = ChessNet(in_channels=encoder_channels, grid_size=grid_size)
#     print('num params:', count_parameters(model))
#     model.eval()
#     n = 5000
#     batch_size = 1
#     X = torch.rand(n, encoder_channels, grid_size, grid_size, device=device)
#     print('shape:', X.shape)
#     tic = time.perf_counter()

#     # Chunked:
#     for x in chunker(X, batch_size):
#         (policy, value) = model(x)

#     toc = time.perf_counter()
#     print('policy, value shape:', policy.shape, value.shape)
#     print('delta:', toc-tic, (toc - tic) / n)


@click.command()
@click.option('-o', '--output', default='conv_4x64.pt')
@click.option('-f', '--force', is_flag=True, help='overwrite')
def main(output, force):
    if not output.endswith('.pt'):
        output += '.pt'
    if not force and (os.path.exists(output) or os.path.exists(output.replace('.pt', '.ts'))):
        raise ValueError('output exists')
    with torch.no_grad():
        grid_size = 8
        encoder_channels = 21
        model = ChessNet(in_channels=encoder_channels)
        model.eval()

        torch.save(model.state_dict(), output)
        X = torch.rand(1, encoder_channels, grid_size, grid_size, device=device)
        traced_script_module = torch.jit.trace(model, X)
        traced_script_module.save(output.replace('.pt', '.ts'))


if __name__ == '__main__':
    main()
