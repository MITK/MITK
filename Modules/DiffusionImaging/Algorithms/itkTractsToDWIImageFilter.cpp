/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkTractsToDWIImageFilter.h"
#include <boost/progress.hpp>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <itkTractDensityImageFilter.h>
#include <itkStatisticsImageFilter.h>
#include <itkTractsToVectorImageFilter.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
    : m_FiberBaseline(200)
    , m_NonFiberBaseline(1000)
{
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);
}

TractsToDWIImageFilter::~TractsToDWIImageFilter()
{

}

void TractsToDWIImageFilter::GenerateData()
{
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    if (m_FiberModels.empty())
        itkExceptionMacro("No diffusion model for fiber compartments defined!");

    if (m_NonFiberModels.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined!");

    int numFibers = m_FiberBundle->GetNumFibers();
    if (numFibers<=0)
        itkExceptionMacro("Input fiber bundle contains no fibers!");

    float minSpacing = 1;
    if(m_Spacing[0]<m_Spacing[1] && m_Spacing[0]<m_Spacing[2])
        minSpacing = m_Spacing[0];
    else if (m_Spacing[1] < m_Spacing[2])
        minSpacing = m_Spacing[1];
    else
        minSpacing = m_Spacing[2];

    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    fiberBundle->DoFiberSmoothing(ceil(10.0*2.0/minSpacing));

    // initialize output dwi image
    OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( m_Origin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
    outImage->Allocate();
    OutputImageType::PixelType temp;
    temp.SetSize(m_FiberModels[0]->GetNumGradients());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);
    this->SetNthOutput(0, outImage);

    // generate working double image because we work with small values (will be scaled later)
    DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
    doubleDwi->SetSpacing( m_Spacing );
    doubleDwi->SetOrigin( m_Origin );
    doubleDwi->SetDirection( m_DirectionMatrix );
    doubleDwi->SetLargestPossibleRegion( m_ImageRegion );
    doubleDwi->SetBufferedRegion( m_ImageRegion );
    doubleDwi->SetRequestedRegion( m_ImageRegion );
    doubleDwi->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
    doubleDwi->Allocate();
    DoubleDwiType::PixelType pix;
    pix.SetSize(m_FiberModels[0]->GetNumGradients());
    pix.Fill(0.0);
    doubleDwi->FillBuffer(pix);

    // search crossing regions
    if (m_TissueMask.IsNull())
    {
        m_TissueMask = ItkUcharImgType::New();
        m_TissueMask->SetSpacing( m_Spacing );
        m_TissueMask->SetOrigin( m_Origin );
        m_TissueMask->SetDirection( m_DirectionMatrix );
        m_TissueMask->SetLargestPossibleRegion( m_ImageRegion );
        m_TissueMask->SetBufferedRegion( m_ImageRegion );
        m_TissueMask->SetRequestedRegion( m_ImageRegion );
        m_TissueMask->Allocate();
        m_TissueMask->FillBuffer(1);
    }
    else
        MITK_INFO << "Using tissue mask";

    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    MITK_INFO << "Generating signal of " << m_FiberModels.size() << " fiber compartments";
    double maxBaseline = 0;
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        ++disp;
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );
        if (numPoints<2)
            continue;

        for( int j=0; j<numPoints; j++)
        {
            double* temp = fiberPolyData->GetPoint(points[j]);
            itk::Point<float, 3> vertex = GetItkPoint(temp);
            vnl_vector<double> v = GetVnlVector(temp);

            vnl_vector<double> dir(3);
            if (j<numPoints-1)
                dir = GetVnlVector(fiberPolyData->GetPoint(points[j+1]))-v;
            else
                dir = v-GetVnlVector(fiberPolyData->GetPoint(points[j-1]));

            itk::Index<3> index;
            outImage->TransformPhysicalPointToIndex(vertex, index);

            if (!m_ImageRegion.IsInside(index))
                continue;

            pix = doubleDwi->GetPixel(index);
            for (int k=0; k<m_FiberModels.size(); k++)
            {
                m_FiberModels[k]->SetFiberDirection(dir);
                doubleDwi->SetPixel(index, pix + m_FiberModels[k]->SimulateMeasurement());
            }
            pix = doubleDwi->GetPixel(index);
            if (pix[0]>maxBaseline)
                maxBaseline = pix[0];
        }
    }

    MITK_INFO << "Adding signal of " << m_NonFiberModels.size() << " non-fiber compartments and noise";
    typedef ImageRegionIterator<DoubleDwiType>      IteratorOutputType;
    IteratorOutputType it (doubleDwi, m_ImageRegion);
    it.GoToBegin();

    DoubleDwiType::PixelType signal;
    signal.SetSize(m_FiberModels[0]->GetNumGradients());
    while(!it.IsAtEnd())
    {
        DoubleDwiType::IndexType index = it.GetIndex();
        signal.Fill(0.0);

        if (m_TissueMask->GetPixel(index)>0)
        {
            pix = it.Get();
            double w = pix[0]/maxBaseline;        // anisotropic volume fraction
            if (pix[0]>0)
                pix /= pix[0];                      // normalize signal

            // add non-fiber compartments
            for (int k=0; k<m_NonFiberModels.size(); k++)
                signal += m_NonFiberModels[k]->SimulateMeasurement();
            if (signal[0]>0)
                signal /= signal[0];
            signal *= (1-w)*m_NonFiberBaseline;
            signal += w*m_FiberBaseline*pix;
        }

        m_NoiseModel->AddNoise(signal);                               // add rician noise
        outImage->SetPixel(index, signal);
        ++it;
    }
}

itk::Point<float, 3> TractsToDWIImageFilter::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(double point[3])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}


vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(Vector<float,3>& vector)
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = vector[0];
    vnlVector[1] = vector[1];
    vnlVector[2] = vector[2];
    return vnlVector;
}

}
