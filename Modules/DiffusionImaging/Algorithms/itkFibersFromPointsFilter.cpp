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
#include "itkFibersFromPointsFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>

// MITK
#include <itkOrientationDistributionFunction.h>
#include <itkDiffusionQballGeneralizedFaImageFilter.h>
#include <mitkStandardFileLocations.h>
#include <mitkFiberBuilder.h>
#include <mitkMetropolisHastingsSampler.h>
//#include <mitkEnergyComputer.h>
#include <itkTensorImageToQBallImageFilter.h>
#include <mitkGibbsEnergyComputer.h>

// ITK
#include <itkImageDuplicator.h>
#include <itkResampleImageFilter.h>
#include <itkTimeProbe.h>

// MISC
#include <fstream>
#include <QFile>
#include <tinyxml.h>
#include <math.h>

namespace itk{

FibersFromPointsFilter::FibersFromPointsFilter()
    : m_Density(1000)
{

}

FibersFromPointsFilter::~FibersFromPointsFilter()
{

}


void FibersFromPointsFilter::GeneratePoints()
{
    m_2DPoints.clear();
    int count = 0;

    while (count < m_Density)
    {
        mitk::Point2D p;
        p[0] = (float)(rand()%2001)/1000 - 1;
        p[1] = (float)(rand()%2001)/1000 - 1;
        if (sqrt(p[0]*p[0]+p[1]*p[1]) <= 1)
        {
            m_2DPoints.push_back(p);
            count++;
        }
    }
}

// perform global tracking
void FibersFromPointsFilter::GenerateData()
{
    for (int i=0; i<m_Fiducials.size(); i++)
    {
        vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints> m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

        vector< mitk::PlanarCircle::Pointer > bundle = m_Fiducials.at(i);

        GeneratePoints();
        for (int j=0; j<m_Density; j++)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

            mitk::PlanarCircle::Pointer figure = bundle.at(0);
            mitk::Point2D p1 = figure->GetControlPoint(0);
            mitk::Point2D p2 = figure->GetControlPoint(1);
            mitk::Vector2D dir = p2-p1; dir.Normalize();
            float r = p1.EuclideanDistanceTo(p2);
            p1[0] += m_2DPoints.at(j)[0]*r;
            p1[1] += m_2DPoints.at(j)[1]*r;

            const mitk::Geometry2D* pfgeometry = figure->GetGeometry2D();
            const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*>(pfgeometry);

            mitk::Point3D w;
            planeGeo->Map(p1, w);

            vtkIdType id = m_VtkPoints->InsertNextPoint(w.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);

            for (int k=1; k<bundle.size(); k++)
            {
                figure = bundle.at(k);
                p1 = figure->GetControlPoint(0);
                p2 = figure->GetControlPoint(1);
                r = p1.EuclideanDistanceTo(p2);

                // rotate vector
                mitk::Vector2D dir2 = p2-p1; dir2.Normalize();
                vnl_matrix_fixed<float, 2, 2> rot;
                rot[0][0] = dir[0]*dir2[0] + dir[1]*dir2[1]; rot[1][1] = rot[0][0];
                rot[1][0] = sin(acos(rot[0][0]));
                rot[0][1] = -sin(rot[1][0]);

                vnl_vector_fixed< float, 2 > newP;
                newP[0] = m_2DPoints.at(j)[0];
                newP[1] = m_2DPoints.at(j)[1];
                newP = rot*newP;

//                vnl_vector_fixed<double, 2> axis = vnl_cross_3d(dir, dir2); axis.normalize();
//                vnl_quaternion<double> rotation(axis, acos(dot_product(dir, dir2)));
//                rotation.normalize();

                p1[0] += newP[0]*r;
                p1[1] += newP[1]*r;

                const mitk::Geometry2D* pfgeometry = figure->GetGeometry2D();
                const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*>(pfgeometry);

                mitk::Point3D w;
                planeGeo->Map(p1, w);

                vtkIdType id = m_VtkPoints->InsertNextPoint(w.GetDataPointer());
                container->GetPointIds()->InsertNextId(id);
            }

            m_VtkCellArray->InsertNextCell(container);
        }

        vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
        fiberPolyData->SetPoints(m_VtkPoints);
        fiberPolyData->SetLines(m_VtkCellArray);
        mitk::FiberBundleX::Pointer mitkFiberBundle = mitk::FiberBundleX::New(fiberPolyData);
        mitkFiberBundle->DoFiberSmoothing(5);
        m_FiberBundles.push_back(mitkFiberBundle);
    }
}

}




