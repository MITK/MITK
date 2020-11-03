/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#define _USE_MATH_DEFINES
#include "mitkHummelProtocolEvaluation.h"

#include <boost/serialization/array_wrapper.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>
#include <boost/math/distributions/normal.hpp>
#include <algorithm>

bool mitk::HummelProtocolEvaluation::Evaluate15cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "15 cm distances are only evaluated for standard volumes, aborting!"; return false; }

  MITK_INFO << "########### 15 cm distance errors #############";

  //convert measurements to matrix
  std::array<std::array<mitk::Point3D, 10> ,9>  matrix = ParseMatrixStandardVolume(p);

  //these are the variables for the results:
  std::vector<double> distances;
  std::vector<std::string> descriptions;

  //evaluation of rows
  int distanceCounter = 0;
  for (int row = 0; row < 9; row++) //rows
    for (int distance = 0; distance < 7; distance++)
    {
      distanceCounter++;
      mitk::Point3D point1 = p->GetPoint(row * 10 + distance);
      mitk::Point3D point2 = p->GetPoint(row * 10 + distance + 3);
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (distance + 1) << " to " << (row + 1) << "/" << (distance + 4);
      descriptions.push_back(description.str());
    }

  //evaluation of columns
  for (int column = 0; column < 10; column++)
    for (int row = 0; row < 6; row++)
    {
      distanceCounter++;
      mitk::Point3D point1 = matrix[row][column];
      mitk::Point3D point2 = matrix[row + 3][column];
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (column + 1) << " to " << (row + 4) << "/" << (column + 1);
      descriptions.push_back(description.str());
    }

  //compute all errors
  for (std::size_t i = 0; i < distances.size(); ++i)
  {
    HummelProtocolDistanceError currentError;
    currentError.distanceError = fabs(distances.at(i) - (double)150.0);
    currentError.description = descriptions.at(i);
    Results.push_back(currentError);
    MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
  }

  //compute statistics
  std::vector<HummelProtocolDistanceError> statistics = mitk::HummelProtocolEvaluation::ComputeStatistics(Results);
  for (auto currentError : statistics)
  {
    Results.push_back(currentError);
    MITK_INFO << currentError.description << " : " << currentError.distanceError;
  }

  return true;
}

bool mitk::HummelProtocolEvaluation::Evaluate30cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "30 cm distances are only evaluated for standard volumes, aborting!"; return false; }
  MITK_INFO << "########### 30 cm distance errors #############";

  //convert measurements to matrix
  std::array<std::array<mitk::Point3D, 10> ,9>  matrix = ParseMatrixStandardVolume(p);

  //these are the variables for the results:
  std::vector<double> distances;
  std::vector<std::string> descriptions;

  //evaluation of rows
  int distanceCounter = 0;
  for (int row = 0; row < 9; row++) //rows
    for (int distance = 0; distance < 4; distance++)
    {
      distanceCounter++;
      mitk::Point3D point1 = p->GetPoint(row * 10 + distance);
      mitk::Point3D point2 = p->GetPoint(row * 10 + distance + 6);
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (distance + 1) << " to " << (row + 1) << "/" << (distance + 7);
      descriptions.push_back(description.str());
    }

  //evaluation of columns
  for (int column = 0; column < 10; column++)
    for (int row = 0; row < 3; row++)
    {
      distanceCounter++;
      mitk::Point3D point1 = matrix[row][column];
      mitk::Point3D point2 = matrix[row + 6][column];
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (column + 1) << " to " << (row + 7) << "/" << (column + 1);
      descriptions.push_back(description.str());
    }

  //compute all errors
  for (std::size_t i = 0; i < distances.size(); ++i)
  {
    HummelProtocolDistanceError currentError;
    currentError.distanceError = fabs(distances.at(i) - (double)300.0);
    currentError.description = descriptions.at(i);
    Results.push_back(currentError);
    MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
  }

  //compute statistics
  std::vector<HummelProtocolDistanceError> statistics = mitk::HummelProtocolEvaluation::ComputeStatistics(Results);
  for (auto currentError : statistics)
  {
    Results.push_back(currentError);
    MITK_INFO << currentError.description << " : " << currentError.distanceError;
  }

  return true;
}

bool mitk::HummelProtocolEvaluation::EvaluateAccumulatedDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "Accumulated distances are only evaluated for standard volumes, aborting!"; return false; }

  MITK_INFO << "########### accumulated distance errors #############";

  int distanceCounter = 0;

  //evaluation of rows
  for (int row = 0; row < 9; row++) //rows
    for (int distance = 0; distance < 9; distance++)
    {
      distanceCounter++;
      mitk::Point3D point1 = p->GetPoint(row * 10);
      mitk::Point3D point2 = p->GetPoint(row * 10 + distance + 1);
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/1 to " << (row + 1) << "/" << (distance + 2);
      //compute error
      HummelProtocolDistanceError currentError;
      currentError.distanceError = fabs(point1.EuclideanDistanceTo(point2) - (double)(50.0*(distance+1)));
      currentError.description = description.str();
      Results.push_back(currentError);
      MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
    }



  //compute statistics
  std::vector<HummelProtocolDistanceError> statistics = mitk::HummelProtocolEvaluation::ComputeStatistics(Results);
  for (auto currentError : statistics)
  {
    Results.push_back(currentError);
    MITK_INFO << currentError.description << " : " << currentError.distanceError;
  }

return true;
}



bool mitk::HummelProtocolEvaluation::Evaluate5cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
MITK_INFO << "########### 5 cm distance errors #############";
std::vector<double> distances;
std::vector<std::string> descriptions;
switch (m)
{
case small:
  if (p->GetSize() != 12) {
  MITK_WARN << "Wrong number of points: " << p->GetSize() << " (expected 12), aborting";
  return false;
  }
  MITK_INFO << "Computing Hummel protocol distance errors for small measurement volumes (12 points)...";

  //row 1
  distances.push_back(p->GetPoint(0).EuclideanDistanceTo(p->GetPoint(1))); //0
  descriptions.push_back("Distance 4/4 to 4/5");
  distances.push_back(p->GetPoint(1).EuclideanDistanceTo(p->GetPoint(2))); //1
  descriptions.push_back("Distance 4/5 to 4/6");
  distances.push_back(p->GetPoint(2).EuclideanDistanceTo(p->GetPoint(3))); //2
  descriptions.push_back("Distance 4/6 to 4/7");
  //row 2
  distances.push_back(p->GetPoint(4).EuclideanDistanceTo(p->GetPoint(5))); //3
  descriptions.push_back("Distance 5/4 to 5/5");
  distances.push_back(p->GetPoint(5).EuclideanDistanceTo(p->GetPoint(6))); //4
  descriptions.push_back("Distance 5/5 to 5/6");
  distances.push_back(p->GetPoint(6).EuclideanDistanceTo(p->GetPoint(7))); //5
  descriptions.push_back("Distance 5/6 to 5/7");
  //row 3
  distances.push_back(p->GetPoint(8).EuclideanDistanceTo(p->GetPoint(9))); //6
  descriptions.push_back("Distance 6/4 to 6/5");
  distances.push_back(p->GetPoint(9).EuclideanDistanceTo(p->GetPoint(10))); //7
  descriptions.push_back("Distance 6/5 to 6/6");
  distances.push_back(p->GetPoint(10).EuclideanDistanceTo(p->GetPoint(11))); //8
  descriptions.push_back("Distance 6/6 to 6/7");
  //column 1
  distances.push_back(p->GetPoint(0).EuclideanDistanceTo(p->GetPoint(4))); //9
  descriptions.push_back("Distance 4/4 to 5/4");
  distances.push_back(p->GetPoint(4).EuclideanDistanceTo(p->GetPoint(8))); //10
  descriptions.push_back("Distance 5/4 to 6/4");
  //column 2
  distances.push_back(p->GetPoint(1).EuclideanDistanceTo(p->GetPoint(5))); //11
  descriptions.push_back("Distance 4/5 to 5/5");
  distances.push_back(p->GetPoint(5).EuclideanDistanceTo(p->GetPoint(9))); //12
  descriptions.push_back("Distance 5/5 to 6/5");
  //column 3
  distances.push_back(p->GetPoint(2).EuclideanDistanceTo(p->GetPoint(6))); //13
  descriptions.push_back("Distance 4/6 to 5/6");
  distances.push_back(p->GetPoint(6).EuclideanDistanceTo(p->GetPoint(10))); //14
  descriptions.push_back("Distance 5/6 to 6/6");
  //column 4
  distances.push_back(p->GetPoint(3).EuclideanDistanceTo(p->GetPoint(7))); //15
  descriptions.push_back("Distance 4/7 to 5/7");
  distances.push_back(p->GetPoint(7).EuclideanDistanceTo(p->GetPoint(11))); //16
  descriptions.push_back("Distance 5/7 to 6/7");

break;

case medium:
{
  if (p->GetSize() != 25) {
    MITK_WARN << "Wrong number of points (expected 25), aborting";
    return false;
  }
  MITK_INFO << "Computing Hummel protocol distance errors for medium measurement volumes (25 points)...";

  int distanceCounter = 0;

  //convert measurements to matrix
  std::array<std::array<mitk::Point3D, 5>, 5>  matrix = ParseMatrixMediumVolume(p);

  //evaluation of rows
  for (int row = 0; row < 5; row++) //rows
    for (int distance = 0; distance < 4; distance++)
    {
      distanceCounter++;
      mitk::Point3D point1 = p->GetPoint(row * 5 + distance);
      mitk::Point3D point2 = p->GetPoint(row * 5 + distance + 1);
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (distance + 1) << " to " << (row + 1) << "/" << (distance + 2);
      descriptions.push_back(description.str());
    }

  //evaluation of columns
  for (int column = 0; column < 5; column++)
    for (int row = 0; row < 4; row++)
    {
      distanceCounter++;
      mitk::Point3D point1 = matrix[row][column];
      mitk::Point3D point2 = matrix[row + 1][column];
      MITK_INFO << "Point 1:" << point1 << "/Point 2:" << point2 << "/Distance:" << point1.EuclideanDistanceTo(point2);
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (column + 1) << " to " << (row + 2) << "/" << (column + 1);
      descriptions.push_back(description.str());
    }
  }
  break;

case standard:
{
  if (p->GetSize() != 90) {
    MITK_WARN << "Wrong number of points (expected 90), aborting";
    return false;
  }
  MITK_INFO << "Computing Hummel protocol distance errors for standard measurement volumes (90 points)...";

  int distanceCounter = 0;

  //convert measurements to matrix
  std::array<std::array<mitk::Point3D, 10>, 9>  matrix = ParseMatrixStandardVolume(p);

  //evaluation of rows
  for (int row = 0; row < 9; row++) //rows
    for (int distance = 0; distance < 9; distance++)
    {
      distanceCounter++;
      mitk::Point3D point1 = p->GetPoint(row * 10 + distance);
      mitk::Point3D point2 = p->GetPoint(row * 10 + distance + 1);
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (distance + 1) << " to " << (row + 1) << "/" << (distance + 2);
      descriptions.push_back(description.str());
    }

  //evaluation of columns
  for (int column = 0; column < 10; column++)
    for (int row = 0; row < 8; row++)
    {
      distanceCounter++;
      mitk::Point3D point1 = matrix[row][column];
      mitk::Point3D point2 = matrix[row + 1][column];
      distances.push_back(point1.EuclideanDistanceTo(point2));
      std::stringstream description;
      description << "Distance(" << distanceCounter << ") " << (row + 1) << "/" << (column + 1) << " to " << (row + 2) << "/" << (column + 1);
      descriptions.push_back(description.str());
    }
  }
  break;


}

//compute all errors
for (std::size_t i = 0; i < distances.size(); ++i)
{
HummelProtocolDistanceError currentError;
currentError.distanceError = fabs(distances.at(i) - (double)50.0);
currentError.description = descriptions.at(i);
Results.push_back(currentError);
MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
}

//compute statistics
std::vector<HummelProtocolDistanceError> statistics = mitk::HummelProtocolEvaluation::ComputeStatistics(Results);
for (auto currentError : statistics)
{
  Results.push_back(currentError);
  MITK_INFO << currentError.description << " : " << currentError.distanceError;
}

return true;
}

std::array<std::array<mitk::Point3D, 10>, 9> mitk::HummelProtocolEvaluation::ParseMatrixStandardVolume(mitk::PointSet::Pointer p)
{

  std::array<std::array<mitk::Point3D, 10> ,9> returnValue;

  if (p->GetSize() != 90)
  {
    MITK_WARN << "PointSet does not have the right size. Expected 90 got " << p->GetSize() << " ... aborting!";
    return returnValue;
  }
  for (int row = 0; row < 9; row++)
    for (int column = 0; column < 10; column++)
      returnValue[row][column] = p->GetPoint(row * 10 + column);
  return returnValue;
}

std::array<std::array<mitk::Point3D, 5>, 5> mitk::HummelProtocolEvaluation::ParseMatrixMediumVolume(mitk::PointSet::Pointer p)
{

  std::array<std::array<mitk::Point3D, 5>, 5> returnValue;

  if (p->GetSize() != 25)
  {
    MITK_WARN << "PointSet does not have the right size. Expected 25 got " << p->GetSize() << " ... aborting!";
    return returnValue;
  }
  for (int row = 0; row < 5; row++)
    for (int column = 0; column < 5; column++)
    {
      returnValue[row][column] = p->GetPoint(row * 5 + column);
      //MITK_INFO << "m " << row << "/" << column << ":" << p->GetPoint(row * 5 + column);
    }

  return returnValue;

}


std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> mitk::HummelProtocolEvaluation::ComputeStatistics(std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> values)
{
  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> returnValue;

  //convert input values to boost / using boost accumulators for statistics
  boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::mean,
                                                                             //boost::accumulators::tag::median,
                                                                             boost::accumulators::tag::variance,
                                                                             boost::accumulators::tag::max,
                                                                             boost::accumulators::tag::min
                                                                             //boost::accumulators::tag::tail<boost::accumulators::left>
                                                                          > > acc;
  for (mitk::HummelProtocolEvaluation::HummelProtocolDistanceError each : values)
  {
    acc(each.distanceError);
  }

  returnValue.push_back({ static_cast<double>(values.size()), "N" });
  returnValue.push_back({ boost::accumulators::mean(acc), "Mean" });
  //double quantile25th = boost::accumulators::quantile(acc, boost::accumulators::quantile_probability = 0.25);
  //returnValue.push_back({ boost::accumulators::median(acc), "Median" });
  //returnValue.push_back({ boost::accumulators::variance(acc), "Variance" });
  returnValue.push_back({ boost::accumulators::min(acc), "Min" });
  returnValue.push_back({ boost::accumulators::max(acc), "Max" });

  //don't get the boost stuff working correctly, so computing the quantiles, median and standard deviation by myself:
  std::vector<double> quantile;
  for (mitk::HummelProtocolEvaluation::HummelProtocolDistanceError each : values)
    {quantile.push_back(each.distanceError);}

  auto const Q1 = quantile.size() / 4;
  auto const Q2 = quantile.size() / 2;
  auto const Q3 = Q1 + Q2;

  std::sort(quantile.begin(),quantile.end());

  returnValue.push_back({ quantile[Q1], "Quartile 1" });
  returnValue.push_back({ quantile[Q2], "Median" });
  returnValue.push_back({ quantile[Q3], "Quartile 3" });

  double mean = boost::accumulators::mean(acc);
  double errorSum = 0;
  for (mitk::HummelProtocolEvaluation::HummelProtocolDistanceError each : values)
  {
    double error = pow((each.distanceError - mean),2);
    errorSum += error;
  }
  double variance = errorSum / values.size();
  double sampleVariance = errorSum / (values.size()-1);
  double standardDev = sqrt(variance);
  double sampleStandardDev = sqrt(sampleVariance);

  returnValue.push_back({ variance, "Variance" });
  returnValue.push_back({ sampleVariance, "Sample Variance" });
  returnValue.push_back({ standardDev, "Standard Deviation" });
  returnValue.push_back({ sampleStandardDev, "Sample Standard Deviation" });


  return returnValue;

}
