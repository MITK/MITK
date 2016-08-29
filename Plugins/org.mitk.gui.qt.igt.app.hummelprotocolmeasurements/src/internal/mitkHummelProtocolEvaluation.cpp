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

bool mitk::HummelProtocolEvaluation::Evaluate5cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results)
{
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

for (int row = 0; row < 10; row++) //rows
for (int distance = 0; distance < 9; distance++)
{
mitk::Point3D point1 = p->GetPoint(row*10 + distance);
mitk::Point3D point2 = p->GetPoint(row*10 + distance+1);
distances.push_back(point1.EuclideanDistanceTo(point2));
std::stringstream description;
description << "Error: Distance " << (row + 1) << "/" << (distance + 1) << " to " << (row + 1) << "/" << (distance + 2);
descriptions.push_back("Error: Distance 5/7 to 6/7");
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
