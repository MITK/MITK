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
    mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();
    typename OutputImageType::Pointer outImage = this->GetOutput();

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

    MITK_INFO << "TractDensityImageFilter: resampling fibers to ensure sufficient voxel coverage";
    m_FiberBundle = m_FiberBundle->GetDeepCopy();
    m_FiberBundle->ResampleSpline(minSpacing/10);

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

        // fill output image
        for( int j=0; j<numPoints; j++)
        {
            itk::Point<float, 3> vertex = GetItkPoint(fiberPolyData->GetPoint(points[j]));
            itk::Index<3> index;
            itk::ContinuousIndex<float, 3> contIndex;
            outImage->TransformPhysicalPointToIndex(vertex, index);
            outImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

            if (!m_UseTrilinearInterpolation && outImage->GetLargestPossibleRegion().IsInside(index))
            {
                if (m_BinaryOutput)
                    outImage->SetPixel(index, 1);
                else
                    outImage->SetPixel(index, outImage->GetPixel(index)+0.01);
                continue;
            }

            float frac_x = contIndex[0] - index[0];
            float frac_y = contIndex[1] - index[1];
            float frac_z = contIndex[2] - index[2];

            if (frac_x<0)
            {
                index[0] -= 1;
                frac_x += 1;
            }
            if (frac_y<0)
            {
                index[1] -= 1;
                frac_y += 1;
            }
            if (frac_z<0)
            {
                index[2] -= 1;
                frac_z += 1;
            }

            frac_x = 1-frac_x;
            frac_y = 1-frac_y;
            frac_z = 1-frac_z;

            // int coordinates inside image?
            if (index[0] < 0 || index[0] >= w-1)
                continue;
            if (index[1] < 0 || index[1] >= h-1)
                continue;
            if (index[2] < 0 || index[2] >= d-1)
                continue;

            if (m_BinaryOutput)
            {
                outImageBufferPointer[( index[0]   + w*(index[1]  + h*index[2]  ))] = 1;
                outImageBufferPointer[( index[0]   + w*(index[1]+1+ h*index[2]  ))] = 1;
                outImageBufferPointer[( index[0]   + w*(index[1]  + h*index[2]+h))] = 1;
                outImageBufferPointer[( index[0]   + w*(index[1]+1+ h*index[2]+h))] = 1;
                outImageBufferPointer[( index[0]+1 + w*(index[1]  + h*index[2]  ))] = 1;
                outImageBufferPointer[( index[0]+1 + w*(index[1]  + h*index[2]+h))] = 1;
                outImageBufferPointer[( index[0]+1 + w*(index[1]+1+ h*index[2]  ))] = 1;
                outImageBufferPointer[( index[0]+1 + w*(index[1]+1+ h*index[2]+h))] = 1;
            }
            else
            {
                outImageBufferPointer[( index[0]   + w*(index[1]  + h*index[2]  ))] += (  frac_x)*(  frac_y)*(  frac_z);
                outImageBufferPointer[( index[0]   + w*(index[1]+1+ h*index[2]  ))] += (  frac_x)*(1-frac_y)*(  frac_z);
                outImageBufferPointer[( index[0]   + w*(index[1]  + h*index[2]+h))] += (  frac_x)*(  frac_y)*(1-frac_z);
                outImageBufferPointer[( index[0]   + w*(index[1]+1+ h*index[2]+h))] += (  frac_x)*(1-frac_y)*(1-frac_z);
                outImageBufferPointer[( index[0]+1 + w*(index[1]  + h*index[2]  ))] += (1-frac_x)*(  frac_y)*(  frac_z);
                outImageBufferPointer[( index[0]+1 + w*(index[1]  + h*index[2]+h))] += (1-frac_x)*(  frac_y)*(1-frac_z);
                outImageBufferPointer[( index[0]+1 + w*(index[1]+1+ h*index[2]  ))] += (1-frac_x)*(1-frac_y)*(  frac_z);
                outImageBufferPointer[( index[0]+1 + w*(index[1]+1+ h*index[2]+h))] += (1-frac_x)*(1-frac_y)*(1-frac_z);
            }
        }
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
                outImageBufferPointer[i] /= max;
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
