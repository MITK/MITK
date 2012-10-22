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
{

}

FibersFromPointsFilter::~FibersFromPointsFilter()
{

}

// perform global tracking
void FibersFromPointsFilter::GenerateData()
{
    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> m_VtkPoints = vtkSmartPointer<vtkPoints>::New();
    int mult = 10;

    for (int i=0; i<m_Fiducials.size(); i++)
    {
        for (int j=0; j<mult; j++)
        {
//            vnl_vector_fixed<double, 3> axis = vnl_cross_3d(kernelDir, dir); axis.normalize();
            float scale = j*1.0/mult;

            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            vector< mitk::PlanarCircle::Pointer > line = m_Fiducials.at(i);
            for (int k=0; k<line.size(); k++)
            {
                mitk::PlanarCircle::Pointer figure = line.at(k);
                mitk::Point3D c = figure->GetWorldControlPoint(0);
                mitk::Point3D r = figure->GetWorldControlPoint(1);

                 mitk::Vector3D dir = r-c;
                 dir *= scale;
                 c += dir;

                vtkIdType id = m_VtkPoints->InsertNextPoint(c.GetDataPointer());
                container->GetPointIds()->InsertNextId(id);
            }
            m_VtkCellArray->InsertNextCell(container);
        }
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(m_VtkPoints);
    fiberPolyData->SetLines(m_VtkCellArray);
    m_FiberBundle = mitk::FiberBundleX::New(fiberPolyData);
    m_FiberBundle->DoFiberSmoothing(5);
}

}




