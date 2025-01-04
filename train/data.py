import glob
import json
import os

import numpy as np
import torch
from torch.utils.data import ConcatDataset, Dataset, Subset

DATATYPES = {"float32": np.float32, "int8": np.int8, "int16": np.int16}


class ExperienceSubset(Subset):
    """Random subset of items from all experience files in a directory."""

    def __init__(self, directory, n):
        dataset = ExperienceSet(directory)
        print(f"loaded experience data with {len(dataset)} moves")
        if n <= 1:
            # Treat as fraction
            n = int(len(dataset) * n)
        if n > len(dataset):
            raise ValueError
        print(f"using subset of {n} moves")
        indices = np.random.choice(len(dataset), n, replace=False)
        super().__init__(dataset, indices)


class ExperienceSet(ConcatDataset):
    """Concatenate all experience files in a given directory."""

    def __init__(self, directory):
        labels = [
            s.split("states")[1].split(".json")[0]
            for s in glob.glob(os.path.join(directory, "states*.json"))
        ]
        triples = [
            [
                os.path.join(directory, f"{key}{label}.json")
                for key in ("states", "rewards", "visit_counts")
            ]
            for label in labels
        ]
        datasets = [ExperienceChunk(*triple) for triple in triples]
        super().__init__(datasets)


class ExperienceChunk(Dataset):
    """Experience data for an individual item.

    Intent is for these to be collected using ChainDataSet.
    """

    def __init__(self, states_path, rewards_path, visit_counts_path):
        """Args:
        path (str): Path to states json file.
        """
        self.states_path = states_path
        self.rewards_path = rewards_path
        self.visit_counts_path = visit_counts_path
        self.dirname = os.path.dirname(states_path)
        assert (
            self.dirname
            == os.path.dirname(rewards_path)
            == os.path.dirname(visit_counts_path)
        )

        with open(states_path) as f:
            self.states_info = json.load(f)
        with open(rewards_path) as f:
            self.rewards_info = json.load(f)
        with open(visit_counts_path) as f:
            self.visit_counts_info = json.load(f)

        self.num_moves = self.states_info["shape"][0]
        # print('loading chunk with', self.num_moves, 'moves')
        strides = self.states_info["strides"]
        assert strides[0] > strides[1] > strides[2] > strides[3]

    def _memmap_data(self, info):
        return np.memmap(
            os.path.join(self.dirname, info["data"]),
            dtype=DATATYPES[info["dtype"]],
            mode="r",
            shape=tuple(info["shape"]),
        )

    def __len__(self):
        return self.num_moves

    def __getitem__(self, idx):
        memmaps = [
            self._memmap_data(info)
            for info in (self.states_info, self.rewards_info, self.visit_counts_info)
        ]

        return [torch.tensor(m[idx]) for m in memmaps]


if __name__ == "__main__":
    # data = ExperienceChunk(
    #     *[f"example/{k}_1.json" for k in ("states", "rewards", "visit_counts")]
    # )
    # data_aug = AugmentedExperienceChunk(
    #     *[f"example/{k}_1.json" for k in ("states", "rewards", "visit_counts")]
    # )
    data = ExperienceSet("experience")
    # data = ExperienceSubset('example', 5)
