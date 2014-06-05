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
#ifndef mitkToFTestingCOMMON_H
#define mitkToFTestingCOMMON_H

#include <MitkToFProcessingExports.h>
#include "mitkNumericTypes.h"
#include <mitkToFProcessingCommon.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <itksys/SystemTools.hxx>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace mitk
{
class MitkToFProcessing_EXPORT ToFTestingCommon
{
public:

    /**
     * @brief PointSetsEqual Method two test if two point sets contain the same points. mitk::Equal is used for comparison of the points.
     * @param pointSet1
     * @param pointSet2
     * @return True if pointsets are equal.
     */
    static bool PointSetsEqual(mitk::PointSet::Pointer pointSet1, mitk::PointSet::Pointer pointSet2)
    {
        bool pointSetsEqual = true;
        if (pointSet1->GetSize()==pointSet2->GetSize())
        {
            for (int i=0; i<pointSet1->GetSize(); i++)
            {
                mitk::Point3D expectedPoint = pointSet1->GetPoint(i);
                mitk::Point3D resultPoint = pointSet2->GetPoint(i);
                if (!mitk::Equal(expectedPoint,resultPoint))
                {
                    std::cout << std::endl;
                    std::cout << std::setprecision(12) << "expected: " << expectedPoint;
                    std::cout << std::endl;
                    std::cout << std::setprecision(12) << "resultPoint: " << resultPoint;
                    std::cout << std::endl;
                    pointSetsEqual = false;
                }
            }
        }
        else
        {
            pointSetsEqual = false;
            MITK_INFO<<"Point sets have different size: "<<pointSet1->GetSize()<<" vs. "<<pointSet2->GetSize();
        }
        return pointSetsEqual;
    }

    /**
     * @brief VtkPolyDatasEqual Convenience method for comparing the points of two vtkPolyData (using PointSetsEqual).
     * @param poly1
     * @param poly2
     * @return True if polydatas are equal.
     */
    static bool VtkPolyDatasEqual( vtkSmartPointer<vtkPolyData> poly1, vtkSmartPointer<vtkPolyData> poly2  )
    {
        return PointSetsEqual(VtkPolyDataToMitkPointSet(poly1), VtkPolyDataToMitkPointSet(poly2));
    }

    /**
     * @brief VtkPolyDataToMitkPointSet Converts a vtkPolyData into an mitkPointSet
     * @param poly Input polydata.
     * @return mitk::PointSet::Pointer The resulting point set.
     */
    static mitk::PointSet::Pointer VtkPolyDataToMitkPointSet( vtkSmartPointer<vtkPolyData> poly )
    {
        mitk::PointSet::Pointer result = mitk::PointSet::New();
        int numberOfPoints = poly->GetNumberOfPoints();
        for (int i=0; i<numberOfPoints; i++)
        {
          double* currentPoint = poly->GetPoint(i);
          mitk::Point3D point;
          point[0] = currentPoint[0];
          point[1] = currentPoint[1];
          point[2] = currentPoint[2];
          result->InsertPoint(i,point);
        }
        return result;
    }
};

}
#endif
