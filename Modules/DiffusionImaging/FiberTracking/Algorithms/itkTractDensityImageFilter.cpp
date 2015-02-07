/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Coindex[1]right (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "itkTractDensityImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCell.h>

// misc
#include <math.h>
#include <boost/progress.hpp>

namespace itk{

template< class OutputImageType >
TractDensityImageFilter< OutputImageType >::TractDensityImageFilter()
    : m_InvertImage(false)
    , m_FiberBundle(NULL)
    , m_UpsamplingFactor(1)
    , m_InputImage(NULL)
    , m_BinaryOutput(false)
    , m_UseImageGeometry(false)
    , m_OutputAbsoluteValues(false)
    , m_UseTrilinearInterpolation(false)
    , m_DoFiberResampling(true)
{

}

template< class OutputImageType >
TractDensityImageFilter< OutputImageType >::~TractDensityImageFilter()
{
}

template< class OutputImageType >
itk::Point<float, 3> TractDensityImageFilter< OutputImageType >::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

template< class OutputImageType >
void TractDensityImageFilter< OutputImageType >::GenerateData()
{
    // generate upsampled image
    typename OutputImageType::Pointer outImage = this->GetOutput();
    if (fibs.empty() && m_FiberBundle.IsNotNull())
        fibs.push_back(m_FiberBundle);

    // calculate new image parameters
    itk::Vector<double,3> newSpacing;
    mitk::Point3D newOrigin;
    itk::Matrix<double, 3, 3> newDirection;
    ImageRegion<3> upsampledRegion;
    if (m_UseImageGeometry && !m_InputImage.IsNull())
    {
        MITK_INFO << "TractDensityImageFilter: using image geometry";
        newSpacing = m_InputImage->GetSpacing()/m_UpsamplingFactor;
        upsampledRegion = m_InputImage->GetLargestPossibleRegion();
        newOrigin = m_InputImage->GetOrigin();
        typename OutputImageType::RegionType::SizeType size = upsampledRegion.GetSize();
        size[0] *= m_UpsamplingFactor;
        size[1] *= m_UpsamplingFactor;
        size[2] *= m_UpsamplingFactor;
        upsampledRegion.SetSize(size);
        newDirection = m_InputImage->GetDirection();
    }
    else
    {
        MITK_INFO << "TractDensityImageFilter: using fiber bundle geometry";
        mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();
        newSpacing = geometry->GetSpacing()/m_UpsamplingFactor;
        newOrigin = geometry->GetOrigin();
        mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();
        newOrigin[0] += bounds.GetElement(0);
        newOrigin[1] += bounds.GetElement(2);
        newOrigin[2] += bounds.GetElement(4);

        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                newDirection[j][i] = geometry->GetMatrixColumn(i)[j];
        upsampledRegion.SetSize(0, geometry->GetExtent(0)*m_UpsamplingFactor);
        upsampledRegion.SetSize(1, geometry->GetExtent(1)*m_UpsamplingFactor);
        upsampledRegion.SetSize(2, geometry->GetExtent(2)*m_UpsamplingFactor);
    }
    typename OutputImageType::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();

    // apply new image parameters
    outImage->SetSpacing( newSpacing );
    outImage->SetOrigin( newOrigin );
    outImage->SetDirection( newDirection );
    outImage->SetLargestPossibleRegion( upsampledRegion );
    outImage->SetBufferedRegion( upsampledRegion );
    outImage->SetRequestedRegion( upsampledRegion );
    outImage->Allocate();
    outImage->FillBuffer(0.0);

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];

    // set/initialize output
    OutPixelType* outImageBufferPointer = (OutPixelType*)outImage->GetBufferPointer();

    // resample fiber bundle
    float minSpacing = 1;
    if(newSpacing[0]<newSpacing[1] && newSpacing[0]<newSpacing[2])
        minSpacing = newSpacing[0];
    else if (newSpacing[1] < newSpacing[2])
        minSpacing = newSpacing[1];
    else
        minSpacing = newSpacing[2];

    for (int f=0; f<fibs.size(); f++)
    {
        typename OutputImageType::Pointer workingImage = OutputImageType::New();
        workingImage->SetSpacing( newSpacing );
        workingImage->SetOrigin( newOrigin );
        workingImage->SetDirection( newDirection );
        workingImage->SetLargestPossibleRegion( upsampledRegion );
        workingImage->SetBufferedRegion( upsampledRegion );
        workingImage->SetRequestedRegion( upsampledRegion );
        workingImage->Allocate();
        workingImage->FillBuffer(0.0);

        m_FiberBundle = fibs.at(f);
        MITK_INFO << "TractDensityImageFilter: resampling fibers to ensure sufficient voxel coverage";
        if (m_DoFiberResampling)
        {
            m_FiberBundle = m_FiberBundle->GetDeepCopy();
            m_FiberBundle->ResampleSpline(minSpacing/10);
        }

        MITK_INFO << "TractDensityImageFilter: starting image generation";

        vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
        vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
        vLines->InitTraversal();
        int numFibers = m_FiberBundle->GetNumFibers();
        boost::progress_display disp(numFibers);
        for( int i=0; i<numFibers; i++ )
        {
            ++disp;
            vtkIdType   numPoints(0);
            vtkIdType*  points(NULL);
            vLines->GetNextCell ( numPoints, points );
            float weight = m_FiberBundle->GetFiberWeight(i);

            // fill output image
            for( int j=0; j<numPoints; j++)
            {
                itk::Point<float, 3> vertex = GetItkPoint(fiberPolyData->GetPoint(points[j]));
                itk::Index<3> index;
                itk::ContinuousIndex<float, 3> contIndex;
                workingImage->TransformPhysicalPointToIndex(vertex, index);
                workingImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

                if (m_BinaryOutput)
                    workingImage->SetPixel(index, 1);
                else
                    workingImage->SetPixel(index, workingImage->GetPixel(index)+0.01*weight);
            }
        }

        OutPixelType* wipImageBufferPointer = (OutPixelType*)workingImage->GetBufferPointer();
        for (int i=0; i<w*h*d; i++)
            if (wipImageBufferPointer[i] > outImageBufferPointer[i])
                outImageBufferPointer[i] = wipImageBufferPointer[i];

    }

    if (!m_OutputAbsoluteValues && !m_BinaryOutput)
    {
        MITK_INFO << "TractDensityImageFilter: max-normalizing output image";
        OutPixelType max = 0;
        for (int i=0; i<w*h*d; i++)
            if (max < outImageBufferPointer[i])
                max = outImageBufferPointer[i];
        if (max>0)
            for (int i=0; i<w*h*d; i++)
            {
                outImageBufferPointer[i] /= max;
            }
    }
    if (m_InvertImage)
    {
        MITK_INFO << "TractDensityImageFilter: inverting image";
        for (int i=0; i<w*h*d; i++)
            outImageBufferPointer[i] = 1-outImageBufferPointer[i];
    }
    MITK_INFO << "TractDensityImageFilter: finished processing";
}
}
