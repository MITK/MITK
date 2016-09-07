/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#define _USE_MATH_DEFINES
#include "mitkHummelProtocolEvaluation.h"

bool mitk::HummelProtocolEvaluation::Evaluate15cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "15 cm distances are only evaluated for standard volumes, aborting!"; return false; }

  MITK_INFO << "########### 15 cm distance errors #############";

  //convert measurements to matrix
  itk::Matrix<itk::Point<double, 3>, 9, 10>  matrix = ParseMatrixStandardVolume(p);

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

  //compute all errors as return value and print them to the console
  for (int i = 0; i < distances.size(); i++)
  {
    HummelProtocolDistanceError currentError;
    currentError.distanceError = abs(distances.at(i) - (double)150.0);
    currentError.description = descriptions.at(i);
    Results.push_back(currentError);
    MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
  }

  return true;
}

bool mitk::HummelProtocolEvaluation::Evaluate30cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "30 cm distances are only evaluated for standard volumes, aborting!"; return false; }
  //convert measurements to matrix
  itk::Matrix<itk::Point<double, 3>, 9, 10>  matrix = ParseMatrixStandardVolume(p);
  return false;
}

bool mitk::HummelProtocolEvaluation::EvaluateAccumulatedDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
  if (m != mitk::HummelProtocolEvaluation::standard) { MITK_WARN << "Accumulated distances are only evaluated for standard volumes, aborting!"; return false; }
  //convert measurements to matrix
  itk::Matrix<itk::Point<double, 3>, 9, 10>  matrix = ParseMatrixStandardVolume(p);
  return false;
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

case standard:
if (p->GetSize() != 90) {
MITK_WARN << "Wrong number of points (expected 90), aborting";
return false;
}
MITK_INFO << "Computing Hummel protocol distance errors for standard measurement volumes (90 points)...";

int distanceCounter = 0;

//convert measurements to matrix
itk::Matrix<itk::Point<double, 3>, 9, 10>  matrix = ParseMatrixStandardVolume(p);

//evaluation of rows
for (int row = 0; row < 9; row++) //rows
  for (int distance = 0; distance < 9; distance++)
  {
  distanceCounter++;
  mitk::Point3D point1 = p->GetPoint(row*10 + distance);
  mitk::Point3D point2 = p->GetPoint(row*10 + distance+1);
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
  mitk::Point3D point2 = matrix[row+1][column];
  distances.push_back(point1.EuclideanDistanceTo(point2));
  std::stringstream description;
  description << "Distance(" << distanceCounter << ") " << (row+1 )<< "/" << (column+1) << " to " << (row + 2) << "/" << (column + 1);
  descriptions.push_back(description.str());
  }

break;
}


HummelProtocolDistanceError meanError;
meanError.distanceError = 0;
meanError.description = "Mean Error";
for (int i = 0; i < distances.size(); i++)
{
HummelProtocolDistanceError currentError;
currentError.distanceError = abs(distances.at(i) - (double)50.0);
currentError.description = descriptions.at(i);
Results.push_back(currentError);
meanError.distanceError += currentError.distanceError;
MITK_INFO << "Error " << currentError.description << " : " << currentError.distanceError;
}
meanError.distanceError /= distances.size();
Results.push_back(meanError);
MITK_INFO << "Mean error : " << meanError.distanceError;

return false;
}

itk::Matrix<itk::Point<double, 3>, 9, 10> mitk::HummelProtocolEvaluation::ParseMatrixStandardVolume(mitk::PointSet::Pointer p)
{
  itk::Matrix<itk::Point<double, 3>, 9, 10> returnValue = itk::Matrix<itk::Point<double, 3>, 9, 10>();
  if (p->GetSize() != 90)
  {
    MITK_WARN << "PointSet does not have the right size. Expected 90 got " << p->GetSize() << " ... aborting!";
    return returnValue;
  }
  for (int row = 0; row < 9; row++)
    for (int column = 0; column < 10; column++)
      returnValue[row][column] = p->GetPoint(row * 10 + column);
}
