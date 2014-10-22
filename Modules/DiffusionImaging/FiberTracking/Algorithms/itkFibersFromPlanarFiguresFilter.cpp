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
#include "itkFibersFromPlanarFiguresFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>

// MITK
#include <itkOrientationDistributionFunction.h>
#include <itkDiffusionQballGeneralizedFaImageFilter.h>
#include <mitkStandardFileLocations.h>
#include <mitkFiberBuilder.h>
#include <mitkMetropolisHastingsSampler.h>
#include <itkTensorImageToQBallImageFilter.h>
#include <mitkGibbsEnergyComputer.h>
#include <mitkRotationOperation.h>
#include <mitkInteractionConst.h>

// ITK
#include <itkImageDuplicator.h>
#include <itkResampleImageFilter.h>
#include <itkTimeProbe.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// MISC
#include <math.h>

namespace itk{

FibersFromPlanarFiguresFilter::FibersFromPlanarFiguresFilter()
{

}

FibersFromPlanarFiguresFilter::~FibersFromPlanarFiguresFilter()
{

}


void FibersFromPlanarFiguresFilter::GeneratePoints()
{
    Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    randGen->SetSeed((unsigned int)0);
    m_2DPoints.clear();
    int count = 0;

    while (count < m_Parameters.m_Density)
    {
        mitk::Vector2D p;
        switch (m_Parameters.m_Distribution) {
            case FiberGenerationParameters::DISTRIBUTE_GAUSSIAN:
                p[0] = randGen->GetNormalVariate(0, m_Parameters.m_Variance);
                p[1] = randGen->GetNormalVariate(0, m_Parameters.m_Variance);
                break;
            default:
                p[0] = randGen->GetUniformVariate(-1, 1);
                p[1] = randGen->GetUniformVariate(-1, 1);
        }

        if (sqrt(p[0]*p[0]+p[1]*p[1]) <= 1)
        {
            m_2DPoints.push_back(p);
            count++;
        }
    }
}

void FibersFromPlanarFiguresFilter::GenerateData()
{
    // check if enough fiducials are available
    for (unsigned int i=0; i<m_Parameters.m_Fiducials.size(); i++)
        if (m_Parameters.m_Fiducials.at(i).size()<2)
            itkExceptionMacro("At least 2 fiducials needed per fiber bundle!");

    for (unsigned int i=0; i<m_Parameters.m_Fiducials.size(); i++)
    {
        vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints> m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

        vector< mitk::PlanarEllipse::Pointer > bundle = m_Parameters.m_Fiducials.at(i);

        vector< unsigned int > fliplist;
        if (i<m_Parameters.m_FlipList.size())
            fliplist = m_Parameters.m_FlipList.at(i);
        else
            fliplist.resize(bundle.size(), 0);
        if (fliplist.size()<bundle.size())
            fliplist.resize(bundle.size(), 0);

        GeneratePoints();
        for (int j=0; j<m_Parameters.m_Density; j++)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

            mitk::PlanarEllipse::Pointer figure = bundle.at(0);
            mitk::Point2D p0 = figure->GetControlPoint(0);
            mitk::Point2D p1 = figure->GetControlPoint(1);
            mitk::Point2D p2 = figure->GetControlPoint(2);
            mitk::Point2D p3 = figure->GetControlPoint(3);
            double r1 = p0.EuclideanDistanceTo(p1);
            double r2 = p0.EuclideanDistanceTo(p2);
            mitk::Vector2D eDir = p1-p0; eDir.Normalize();
            mitk::Vector2D tDir = p3-p0; tDir.Normalize();

            // apply twist
            vnl_matrix_fixed<double, 2, 2> tRot;
            tRot[0][0] = tDir[0];
            tRot[1][1] = tRot[0][0];
            tRot[1][0] = sin(acos(tRot[0][0]));
            tRot[0][1] = -tRot[1][0];
            if (tDir[1]<0)
                tRot.inplace_transpose();
            m_2DPoints[j].SetVnlVector(tRot*m_2DPoints[j].GetVnlVector());

            // apply new ellipse shape
            vnl_vector_fixed< double, 2 > newP;
            newP[0] = m_2DPoints.at(j)[0];
            newP[1] = m_2DPoints.at(j)[1];
            double alpha = acos(eDir[0]);
            if (eDir[1]>0)
                alpha = 2*M_PI-alpha;
            vnl_matrix_fixed<double, 2, 2> eRot;
            eRot[0][0] = cos(alpha);
            eRot[1][1] = eRot[0][0];
            eRot[1][0] = sin(alpha);
            eRot[0][1] = -eRot[1][0];
            newP = eRot*newP;
            newP[0] *= r1;
            newP[1] *= r2;
            newP = eRot.transpose()*newP;

            p0[0] += newP[0];
            p0[1] += newP[1];

            const mitk::PlaneGeometry* planeGeo = figure->GetPlaneGeometry();

            mitk::Point3D w, wc;
            planeGeo->Map(p0, w);

            wc = figure->GetWorldControlPoint(0);

            vtkIdType id = m_VtkPoints->InsertNextPoint(w.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);

            vnl_vector_fixed< double, 3 > n = planeGeo->GetNormalVnl();

            for (unsigned int k=1; k<bundle.size(); k++)
            {
                figure = bundle.at(k);
                p0 = figure->GetControlPoint(0);
                p1 = figure->GetControlPoint(1);
                p2 = figure->GetControlPoint(2);
                p3 = figure->GetControlPoint(3);
                r1 = p0.EuclideanDistanceTo(p1);
                r2 = p0.EuclideanDistanceTo(p2);

                eDir = p1-p0; eDir.Normalize();
                mitk::Vector2D tDir2 = p3-p0; tDir2.Normalize();
                mitk::Vector2D temp; temp.SetVnlVector(tRot.transpose() * tDir2.GetVnlVector());

                // apply twist
                tRot[0][0] = tDir[0]*tDir2[0] + tDir[1]*tDir2[1];
                tRot[1][1] = tRot[0][0];
                tRot[1][0] = sin(acos(tRot[0][0]));
                tRot[0][1] = -tRot[1][0];
                if (temp[1]<0)
                    tRot.inplace_transpose();
                m_2DPoints[j].SetVnlVector(tRot*m_2DPoints[j].GetVnlVector());
                tDir = tDir2;

                // apply new ellipse shape
                newP[0] = m_2DPoints.at(j)[0];
                newP[1] = m_2DPoints.at(j)[1];

                // calculate normal
                mitk::PlaneGeometry* planeGeo = const_cast<mitk::PlaneGeometry*>(figure->GetPlaneGeometry());
                mitk::Vector3D perp = wc-planeGeo->ProjectPointOntoPlane(wc); perp.Normalize();
                vnl_vector_fixed< double, 3 > n2 = planeGeo->GetNormalVnl();
                wc = figure->GetWorldControlPoint(0);

                // is flip needed?
                if (dot_product(perp.GetVnlVector(),n2)>0 && dot_product(n,n2)<=0.00001)
                    newP[0] *= -1;
                if (fliplist.at(k)>0)
                    newP[0] *= -1;
                n = n2;

                alpha = acos(eDir[0]);
                if (eDir[1]>0)
                    alpha = 2*M_PI-alpha;
                eRot[0][0] = cos(alpha);
                eRot[1][1] = eRot[0][0];
                eRot[1][0] = sin(alpha);
                eRot[0][1] = -eRot[1][0];
                newP = eRot*newP;
                newP[0] *= r1;
                newP[1] *= r2;
                newP = eRot.transpose()*newP;

                p0[0] += newP[0];
                p0[1] += newP[1];

                mitk::Point3D w;
                planeGeo->Map(p0, w);


                vtkIdType id = m_VtkPoints->InsertNextPoint(w.GetDataPointer());
                container->GetPointIds()->InsertNextId(id);
            }

            m_VtkCellArray->InsertNextCell(container);
        }

        vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
        fiberPolyData->SetPoints(m_VtkPoints);
        fiberPolyData->SetLines(m_VtkCellArray);
        mitk::FiberBundleX::Pointer mitkFiberBundle = mitk::FiberBundleX::New(fiberPolyData);
        mitkFiberBundle->ResampleSpline(m_Parameters.m_Sampling, m_Parameters.m_Tension, m_Parameters.m_Continuity, m_Parameters.m_Bias);
        m_FiberBundles.push_back(mitkFiberBundle);
    }
}

}




