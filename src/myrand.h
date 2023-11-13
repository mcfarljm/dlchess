#ifndef MYRAND_H
#define MYRAND_H

#include <random>

extern std::default_random_engine rng;


class DirichletDistribution {
  double alpha;
  int n;
  std::gamma_distribution<> gamma_dist;

public:
  DirichletDistribution(int n, double alpha) :
    n(n), alpha(alpha), gamma_dist(std::gamma_distribution<>(alpha)) {}

  std::vector<double> sample();
};


#endif // MYRAND_H
