import os
import sys
import time

import click
import numpy as np
import onnxruntime


THIS_DIR = os.path.abspath(os.path.dirname(__file__))

def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))


@click.command()
@click.option('-n', '--num-samples', default=10000, show_default=True)
@click.option('-w', '--num-warmup', default=25000, show_default=True)
@click.option('-b', '--batch-size', default=1, show_default=True)
@click.option('-t', '--num-threads', default=0, show_default=True)
@click.option('--network')
def main(num_samples, num_warmup, batch_size, num_threads, network):

    if network is None:
        network = os.path.join(THIS_DIR, 'conv_4x64.onnx')

    grid_size = 8
    encoder_channels = 21

    sess_opt = onnxruntime.SessionOptions()
    sess_opt.intra_op_num_threads = num_threads

    ort_session = onnxruntime.InferenceSession(network, sess_opt)

    # Warmup:
    print('Warming up...')
    X = np.random.rand(num_warmup, encoder_channels, grid_size, grid_size).astype(np.float32)

    tic = time.perf_counter()
    for x in chunker(X, batch_size):
        ort_inputs = {ort_session.get_inputs()[0].name: x}
        ort_outs = ort_session.run(None, ort_inputs)
    toc = time.perf_counter()
    print(f"Warmup time: {toc - tic:.3f} s")


    print('Benchmarking...')
    X = np.random.rand(num_samples, encoder_channels, grid_size, grid_size).astype(np.float32)

    tic = time.perf_counter()

    # Chunked:
    for x in chunker(X, batch_size):
        ort_inputs = {ort_session.get_inputs()[0].name: x}
        ort_outs = ort_session.run(None, ort_inputs)

    toc = time.perf_counter()
    print(f"Time: {toc - tic:.3f} s")
    print(f"Throughput: {num_samples / (toc - tic):.2f} per second")


if __name__ == '__main__':
    main()
