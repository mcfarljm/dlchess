import os
import time

import torch
from torch import nn
import click
from torchvision.ops import SqueezeExcitation


class ResidSEBlock(nn.Module):
    def __init__(self, in_channels, out_channels, squeeze_channels):
        super().__init__()
        self.conv1 = nn.Conv2d(
            in_channels, out_channels, kernel_size=3, padding=1, bias=False
        )
        self.bn1 = nn.BatchNorm2d(out_channels)
        self.relu = nn.ReLU(inplace=True)
        self.conv2 = nn.Conv2d(
            out_channels, out_channels, kernel_size=3, padding=1, bias=False
        )
        self.bn2 = nn.BatchNorm2d(out_channels)

        self.shortcut = nn.Sequential()
        if in_channels != out_channels:
            self.shortcut = nn.Sequential(
                nn.Conv2d(in_channels, out_channels, kernel_size=1, bias=False),
                nn.BatchNorm2d(out_channels),
            )

        self.se = SqueezeExcitation(out_channels, squeeze_channels)

    def forward(self, x):
        out = self.relu(self.bn1(self.conv1(x)))
        out = self.bn2(self.conv2(out))
        out = self.se(out)
        out += self.shortcut(x)
        out = self.relu(out)
        return out


class ChessNet(nn.Module):
    def __init__(
        self,
        in_channels=21,
        num_filters=64,
        num_blocks=4,
        squeeze_channels=4,
        grid_size=8,
        input_conv=False,
    ):
        self.in_channels = in_channels
        self.grid_size = grid_size
        super().__init__()

        if input_conv:
            # First convolution, goes from in_channels to num_filters channels
            conv1 = nn.Sequential(
                nn.Conv2d(
                    in_channels, num_filters, kernel_size=3, padding=1, bias=False
                ),
                nn.BatchNorm2d(num_filters),
                nn.ReLU(),
            )

            blocks = [conv1]

        else:
            blocks = [ResidSEBlock(in_channels, num_filters, squeeze_channels)]
            num_blocks -= 1

        blocks.extend(
            [
                ResidSEBlock(num_filters, num_filters, squeeze_channels)
                for _ in range(num_blocks)
            ]
        )
        self.base = nn.Sequential(*blocks)

        self.policy_stack = nn.Sequential(
            nn.Conv2d(num_filters, num_filters, 3, padding=1, bias=False),
            nn.BatchNorm2d(num_filters),
            nn.ReLU(),
            nn.Conv2d(num_filters, 73, 3, padding=1),
        )

        self.value_stack = nn.Sequential(
            nn.Conv2d(num_filters, 1, 1, bias=False),
            nn.BatchNorm2d(1),
            nn.ReLU(),
            nn.Flatten(),
            nn.Linear(grid_size**2, 256),
            nn.ReLU(),
            nn.Linear(256, 1),
            nn.Tanh(),
        )

    def forward(self, x):
        base = self.base(x)
        policy = self.policy_stack(base)
        value = self.value_stack(base)
        return policy, value


def count_parameters(model):
    return sum(p.numel() for p in model.parameters() if p.requires_grad)


def benchmark_model(model):
    def chunker(seq, size):
        return (seq[pos : pos + size] for pos in range(0, len(seq), size))

    with torch.no_grad():
        print("default threads:", torch.get_num_threads())
        # torch.set_num_threads(1)
        grid_size = 8
        print("num params:", count_parameters(model))
        model.eval()
        n = 5000
        batch_size = 1
        X = torch.rand(n, model.in_channels, grid_size, grid_size)
        print("shape:", X.shape)
        tic = time.perf_counter()

        # Chunked:
        for x in chunker(X, batch_size):
            (policy, value) = model(x)

        toc = time.perf_counter()
        print("policy, value shape:", policy.shape, value.shape)
        print("delta:", toc - tic)
        print("eval / s:", n / (toc - tic))


@click.command()
@click.option("-o", "--output", default="squeeze_4x64.pt")
@click.option("-f", "--force", is_flag=True, help="overwrite")
@click.option("-i", "--input", help="input file with model state")
@click.option("-v", "--encoding-version", default=1, show_default=True)
@click.option(
    "-n",
    "--num-parameters",
    is_flag=True,
    help="print number of model parameters and exit",
)
@click.option("-b", "--benchmark", is_flag=True)
@click.option("--input-conv", is_flag=True, help="include convolution before blocks")
def main(output, force, input, encoding_version, num_parameters, benchmark, input_conv):
    grid_size = 8
    encoder_channels = 21 if encoding_version == 0 else 22
    model = ChessNet(in_channels=encoder_channels, input_conv=input_conv)

    if num_parameters:
        print(count_parameters(model))
        return

    if benchmark:
        benchmark_model(model)
        return

    if not output.endswith(".pt"):
        output += ".pt"
    if not force:
        if input is None and (
            os.path.exists(output) or os.path.exists(output.replace(".pt", ".onnx"))
        ):
            raise ValueError("output exists")
        elif input is not None and (os.path.exists(output.replace(".pt", ".onnx"))):
            raise ValueError(".onnx output exists")

    if input is not None:
        model.load_state_dict(torch.load(input))
    else:
        torch.save(model.state_dict(), output)
    model.eval()

    X = torch.rand(1, encoder_channels, grid_size, grid_size, requires_grad=True)
    torch.onnx.export(
        model,
        X,
        output.replace(".pt", ".onnx"),
        input_names=["state"],
        output_names=["policy", "value"],
        dynamic_axes={
            "state": {0: "batch_size"},  # variable length axes
            "policy": {0: "batch_size"},
            "value": {0: "batch_size"},
        },
    )


if __name__ == "__main__":
    main()
