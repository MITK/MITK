#include <mitkHalfLifeConstants.h>

const std::string mitk::HALFLIFECONSTANTS::NAME_18F = "18F";
const std::string mitk::HALFLIFECONSTANTS::NAME_68Ga = "68Ga";
const std::string mitk::HALFLIFECONSTANTS::NAME_11C = "11C";
const std::string mitk::HALFLIFECONSTANTS::NAME_15O = "15O";

// Half-life values in seconds. Source: NNDC NuDat 3 nuclear data search
// (https://www.nndc.bnl.gov/nudat3/), retrieved 2026-04-30. See
// mitkHalfLifeConstants.h for per-constant references and uncertainties.
const double mitk::HALFLIFECONSTANTS::VALUE_18F = 6586.2;   // 109.77 min
const double mitk::HALFLIFECONSTANTS::VALUE_68Ga = 4062.6;  // 67.71 min
const double mitk::HALFLIFECONSTANTS::VALUE_11C = 1221.8;   // 20.363 min
const double mitk::HALFLIFECONSTANTS::VALUE_15O = 122.24;   // 2.0373 min
