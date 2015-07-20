#include <mitkDistModels.h>
#include <cmath>

double mitk::DistSimpleBinominal::Init(double y)
{
  return ((y + 0.5) / (2));
}

double mitk::DistSimpleBinominal::Devariation(double mu, double y)
{
  return 2 * (y * std::log(y) / mu + (1 - y) * std::log(1 - y) / (1 - mu));
}

double mitk::DistSimpleBinominal::SqrtVariance(double mu)
{
  return std::sqrt(mu) * std::sqrt(1 - mu);
}