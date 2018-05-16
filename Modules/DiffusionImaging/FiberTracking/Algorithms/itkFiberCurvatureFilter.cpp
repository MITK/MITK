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

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <boost/progress.hpp>

namespace itk{

FiberCurvatureFilter::FiberCurvatureFilter()
    : m_AngularDeviation(30)
    , m_Distance(10.0)
    , m_RemoveFibers(false)
    , m_UseMedian(false)
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
#pragma omp parallel for
    for (int i=0; i<inputPoly->GetNumberOfCells(); i++)
    {
        std::vector< vnl_vector_fixed< double, 3 > > vertices;

#pragma omp critical
        {
            ++disp;
            vtkCell* cell = inputPoly->GetCell(i);
            int numPoints = cell->GetNumberOfPoints();
            vtkPoints* points = cell->GetPoints();

            for (int j=0; j<numPoints; j++)
            {
                double p[3];
                points->GetPoint(j, p);
                vnl_vector_fixed< double, 3 > p_vec;
                p_vec[0]=p[0]; p_vec[1]=p[1]; p_vec[2]=p[2];
                vertices.push_back(p_vec);
            }
        }

        // calculate curvatures
        int numPoints = vertices.size();
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double dist = 0;
            int c = j;
            std::vector< vnl_vector_fixed< double, 3 > > vectors;
            vnl_vector_fixed< double, 3 > meanV; meanV.fill(0.0);
            while(dist<m_Distance/2 && c>1)
            {
              vnl_vector_fixed< double, 3 > p1 = vertices.at(c-1);
              vnl_vector_fixed< double, 3 > p2 = vertices.at(c);

                vnl_vector_fixed< double, 3 > v = p2-p1;
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (m_UseMedian && c==j)
                    meanV += v;
                else if (!m_UseMedian)
                  meanV += v;
                c--;
            }
            c = j;
            dist = 0;
            while(dist<m_Distance/2 && c<numPoints-1)
            {
              vnl_vector_fixed< double, 3 > p1 = vertices.at(c);
              vnl_vector_fixed< double, 3 > p2 = vertices.at(c+1);

                vnl_vector_fixed< double, 3 > v = p2-p1;
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (m_UseMedian && c==j)
                    meanV += v;
                else if (!m_UseMedian)
                  meanV += v;
                c++;
            }
            meanV.normalize();

            double dev = 0;
            for (auto vec : vectors)
            {
                double angle = dot_product(meanV, vec);
                if (angle>1.0)
                    angle = 1.0;
                if (angle<-1.0)
                    angle = -1.0;
                dev += acos(angle)*180/itk::Math::pi;
            }
            if (vectors.size()>0)
                dev /= vectors.size();

            if (dev<m_AngularDeviation)
            {
#pragma omp critical
              {
                vtkIdType id = vtkNewPoints->InsertNextPoint(vertices.at(j).data_block());
                container->GetPointIds()->InsertNextId(id);
              }
            }
            else
            {
                if (m_RemoveFibers)
                {
                    container = vtkSmartPointer<vtkPolyLine>::New();
                    break;
                }

                if (container->GetNumberOfPoints()>0)
                {
#pragma omp critical
                    vtkNewCells->InsertNextCell(container);
                }
                container = vtkSmartPointer<vtkPolyLine>::New();
            }
        }

#pragma omp critical
        {
          if (container->GetNumberOfPoints()>0)
              vtkNewCells->InsertNextCell(container);
        }
    }

    vtkSmartPointer<vtkPolyData> outputPoly = vtkSmartPointer<vtkPolyData>::New();
    outputPoly->SetPoints(vtkNewPoints);
    outputPoly->SetLines(vtkNewCells);
    m_OutputFiberBundle = FiberBundle::New(outputPoly);
}

}




