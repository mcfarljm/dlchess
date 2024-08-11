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
            nn.Conv2d(in_channels, 64, 3, padding=1, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding=1, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding=1, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 64, 3, padding=1, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),
        )

        self.policy_stack = nn.Sequential(
            nn.Conv2d(64, 64, 3, padding=1, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(),

            nn.Conv2d(64, 73, 3, padding=1),
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
@click.option('-i', '--input', help='input file with model state')
@click.option('-v', '--encoding-version', default=1, show_default=True)
@click.option("-n", "--num-parameters", is_flag=True,
              help="print number of model parameters and exit")
def main(output, force, input, encoding_version, num_parameters):
    grid_size = 8
    encoder_channels = 21 if encoding_version == 0 else 22
    model = ChessNet(in_channels=encoder_channels)

    if num_parameters:
        print(count_parameters(model))
        return

    if not output.endswith('.pt'):
        output += '.pt'
    if not force:
        if input is None and (os.path.exists(output) or os.path.exists(output.replace('.pt', '.onnx'))):
            raise ValueError('output exists')
        elif input is not None and (os.path.exists(output.replace('.pt', '.onnx'))):
            raise ValueError('.onnx output exists')

    if input is not None:
        model.load_state_dict(torch.load(input))
    else:
        torch.save(model.state_dict(), output)
    model.eval()

    # Noting that online examples with ONNX export do set requires_grad=True
    # on the sample input, but not sure if it is necessary.
    X = torch.rand(1, encoder_channels, grid_size, grid_size,
                   requires_grad=True, device=device)
    # Note that the exported model has fixed shapes for input and output.
    # This should be OK as long as inference is done in batches of 1.  If
    # needed, we can add a dynamic_axes option to make the first axis (of
    # input and outputs) dynamic.
    torch.onnx.export(model, X,
                      output.replace('.pt', '.onnx'),
                      input_names=['state'],
                      output_names=['policy', 'value'],
                      # dynamic_axes={'state' : {0 : 'batch_size'},    # variable length axes
                      #               'policy' : {0 : 'batch_size'},
                      #               'value': {0: 'batch_size'}}
                      )
    # print(torch.onnx.export_to_pretty_string(
    #     model, X,
    #     input_names=['state'],
    #     output_names=['policy', 'value'],
    # ))


if __name__ == '__main__':
    main()
