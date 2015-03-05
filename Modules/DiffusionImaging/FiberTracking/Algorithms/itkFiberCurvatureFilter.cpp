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
#include "itkFiberCurvatureFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <boost/progress.hpp>

namespace itk{

FiberCurvatureFilter::FiberCurvatureFilter()
    : m_AngularDeviation(30)
    , m_Distance(5.0)
    , m_RemoveFibers(false)
{

}

FiberCurvatureFilter::~FiberCurvatureFilter()
{

}

void FiberCurvatureFilter::GenerateData()
{
    vtkSmartPointer<vtkPolyData> inputPoly = m_InputFiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Applying curvature threshold";
    boost::progress_display disp(inputPoly->GetNumberOfCells());
    for (int i=0; i<inputPoly->GetNumberOfCells(); i++)
    {
        ++disp;
        vtkCell* cell = inputPoly->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        // calculate curvatures
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double dist = 0;
            int c = j;
            std::vector< vnl_vector_fixed< float, 3 > > vectors;
            vnl_vector_fixed< float, 3 > meanV; meanV.fill(0.0);
            while(dist<m_Distance/2 && c>1)
            {
                double p1[3];
                points->GetPoint(c-1, p1);
                double p2[3];
                points->GetPoint(c, p2);

                vnl_vector_fixed< float, 3 > v;
                v[0] = p2[0]-p1[0];
                v[1] = p2[1]-p1[1];
                v[2] = p2[2]-p1[2];
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (c==j)
                    meanV += v;
                c--;
            }
            c = j;
            dist = 0;
            while(dist<m_Distance/2 && c<numPoints-1)
            {
                double p1[3];
                points->GetPoint(c, p1);
                double p2[3];
                points->GetPoint(c+1, p2);

                vnl_vector_fixed< float, 3 > v;
                v[0] = p2[0]-p1[0];
                v[1] = p2[1]-p1[1];
                v[2] = p2[2]-p1[2];
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (c==j)
                    meanV += v;
                c++;
            }
            meanV.normalize();

            double dev = 0;
            for (int c=0; c<vectors.size(); c++)
            {
                double angle = dot_product(meanV, vectors.at(c));
                if (angle>1.0)
                    angle = 1.0;
                if (angle<-1.0)
                    angle = -1.0;
                dev += acos(angle)*180/M_PI;
            }
            if (vectors.size()>0)
                dev /= vectors.size();

            if (dev<m_AngularDeviation)
            {
                double p[3];
                points->GetPoint(j, p);
                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                container->GetPointIds()->InsertNextId(id);
            }
            else
            {
                if (m_RemoveFibers)
                {
                    container = vtkSmartPointer<vtkPolyLine>::New();
                    break;
                }

                if (container->GetNumberOfPoints()>0)
                    vtkNewCells->InsertNextCell(container);
                container = vtkSmartPointer<vtkPolyLine>::New();
            }
        }
        if (container->GetNumberOfPoints()>0)
            vtkNewCells->InsertNextCell(container);
    }

    vtkSmartPointer<vtkPolyData> outputPoly = vtkSmartPointer<vtkPolyData>::New();
    outputPoly->SetPoints(vtkNewPoints);
    outputPoly->SetLines(vtkNewCells);
    m_OutputFiberBundle = FiberBundle::New(outputPoly);
}

}




