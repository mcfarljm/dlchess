#include "myrand.h"

std::default_random_engine rng(std::random_device{}());

std::vector<double> DirichletDistribution::sample() {
  std::vector<double> samples;
  double sum = 0.0;
  for (int i=0; i<n; ++i) {
    double sample = gamma_dist(rng);
    sum += sample;
    samples.push_back(sample);
  }

  for (auto& x : samples)
    x /= sum;

  return samples;
}
