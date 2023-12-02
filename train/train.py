import os

import torch
from torch.utils.data import DataLoader
import click

from data import ExperienceSubset


def cross_entropy_loss_fn(policies, visit_counts):
    search_probs = visit_counts / visit_counts.sum((1, 2, 3), keepdims=True)
    assert search_probs[0].sum() > 0.999 and search_probs[0].sum() < 1.001

    # We divide by the number of rows (examples) to get a mean of the
    # cross-entropy loss.
    return -(search_probs * torch.log(policies)).sum() / len(search_probs)


def train(dataloader, model, optimizer, output_interval):
    mse_loss_fn = torch.nn.MSELoss()
    size = len(dataloader.dataset)
    num_batches = len(dataloader)
    softmax = torch.nn.Softmax(1)
    model.train()
    for batch_num, (states, rewards, visit_counts) in enumerate(dataloader):

        # Compute prediction error
        policy_raw, values = model(states)
        policy_probs = softmax(policy_raw.view(x.shape[0], -1)).view_as(policy_raw)

        mse_loss = mse_loss_fn(values.squeeze(), rewards)
        cross_entropy_loss = cross_entropy_loss_fn(policy_probs, visit_counts)

        loss = mse_loss + cross_entropy_loss

        # Backpropagation
        loss.backward()
        optimizer.step()
        optimizer.zero_grad()

        if batch_num % output_interval == 0:
            loss, current = loss.item(), (batch_num + 1) * len(states)
            mse_loss, ce_loss = mse_loss.item(), cross_entropy_loss.item()
            print(f"loss: {loss:>7f} ({mse_loss:>3f}, {ce_loss:>3f})  [{current:>5d}/{size:>5d}] [{batch_num + 1}/{num_batches}]")


@click.command()
@click.option('-e', '--experience', required=True)
@click.option('-q', '--query', is_flag=True, help='query experience')
@click.option('-b', '--batch-size', type=int, default=256)
@click.option('-i', '--input-path', help='path to input parameter file')
@click.option('-o', '--output-path')
@click.option('-n', '--subset', default=1.0, help='number or fraction of examples to use')
@click.option('--lr', default=1e-2, help='learning rate')
@click.option('--interval', default=1, help='output interval')
@click.option('-f', '--force', is_flag=True, help='overwrite existing output files')
def main(experience, query, batch_size, input_path, output_path, subset, lr, interval, force):
    THIS_DIR = os.path.abspath(os.path.dirname(__file__))

    import sys
    sys.path.append(os.path.join(THIS_DIR, '../nn'))
    from conv_4x64 import ChessNet

    if int(subset) == subset:
        subset = int(subset)

    if output_path and (not force) and os.path.exists(output_path):
        raise ValueError('output path exists')

    dataset = ExperienceSubset(experience, subset)
    dataloader = DataLoader(dataset, batch_size)
    if query:
        return
    
    model = ChessNet()
    model.load_state_dict(torch.load(input_path))
    
    optimizer = torch.optim.SGD(model.parameters(),
                                # Paper starts with 1e-2
                                lr=lr,
                                momentum=0.9,
                                weight_decay=1e-4)

    train(dataloader, model, optimizer, interval)

    if output_path:
        torch.save(model.state_dict(), output_path)

        # Torchscript
        model.eval()
        X = torch.rand(1, model.in_channels, model.grid_size, model.grid_size)
        traced_script_module = torch.jit.trace(model, X)
        traced_script_module.save(output_path.replace('.pt', '.ts'))


if __name__ == '__main__':
    main()
