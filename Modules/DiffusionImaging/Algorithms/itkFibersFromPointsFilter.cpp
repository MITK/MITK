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
    : m_Density(100)
{

}

FibersFromPointsFilter::~FibersFromPointsFilter()
{

}

// perform global tracking
void FibersFromPointsFilter::GenerateData()
{
    for (int i=0; i<m_Fiducials.size(); i++)
    {
        vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints> m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

        vector< mitk::Point3D > centerPoints;
        vector< mitk::Vector3D > directions;
        vector< mitk::Vector3D > rotationAxes;
        vector< float > radii;

        vector< mitk::PlanarCircle::Pointer > bundle = m_Fiducials.at(i);
        for (int k=0; k<bundle.size(); k++)
        {
            mitk::PlanarCircle::Pointer figure = bundle.at(k);  // fiducial k on bundle i
            mitk::Point3D c = figure->GetWorldControlPoint(0);  // center point
            mitk::Point3D r = figure->GetWorldControlPoint(1);  // border point
            mitk::Vector3D dir = r-c;                           // radius vector
            float radius = c.EuclideanDistanceTo(r);            // radius
            const mitk::Geometry2D* pfgeometry = figure->GetGeometry2D();
            const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*>(pfgeometry);
            mitk::Vector3D axis = planeGeo->GetNormal();        // rotation axis
            centerPoints.push_back(c);
            directions.push_back(dir);
            rotationAxes.push_back(axis);
            radii.push_back(radius);
        }

        srand ( time(NULL) );
        for (int j=0; j<=m_Density; j++)
        {
            float rscale = j*1.0/m_Density;

            int onShell = rscale*m_Density;
            int offset = rand()%360;

            for (int l=0; l<onShell; l++)
            {
                float angle = offset + l*360.0/onShell;

                vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                for (int k=0; k<bundle.size(); k++)
                {
                    mitk::Point3D c = centerPoints.at(k);               // center point
                    mitk::Vector3D dir = directions.at(k);              // radius vector

                    if (j>1)
                    {
                        dir.Normalize();
                        float radius = radii.at(k);          // radius of fiducial k in bundle i

                        mitk::Vector3D axis = rotationAxes.at(k);            // center axis
                        vnl_quaternion<float> rotation(axis.GetVnlVector(), angle);
                        rotation.normalize();
                        dir.SetVnlVector(rotation.rotate(dir.GetVnlVector()));  // rotate

                        dir *= rscale*radius; c += dir;                          // shift outward
                    }

                    vtkIdType id = m_VtkPoints->InsertNextPoint(c.GetDataPointer());
                    container->GetPointIds()->InsertNextId(id);
                }
                m_VtkCellArray->InsertNextCell(container);
            }
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




