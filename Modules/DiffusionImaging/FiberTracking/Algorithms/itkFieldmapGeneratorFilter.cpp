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
#include "itkFieldmapGeneratorFilter.h"

#include <itkImageRegionIterator.h>
#include <math.h>
#include <itkPoint.h>

namespace itk{

template< class OutputImageType >
FieldmapGeneratorFilter< OutputImageType >::FieldmapGeneratorFilter()
{

}

template< class OutputImageType >
FieldmapGeneratorFilter< OutputImageType >::~FieldmapGeneratorFilter()
{
}

template< class OutputImageType >
void FieldmapGeneratorFilter< OutputImageType >::BeforeThreadedGenerateData()
{
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( m_Origin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->Allocate();
    outImage->FillBuffer(0);
    this->SetNthOutput(0, outImage);
}

template< class OutputImageType >
void FieldmapGeneratorFilter< OutputImageType >::ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId)
{
    typename OutputImageType::Pointer outImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    ImageRegionIterator< OutputImageType > oit(outImage, outputRegionForThread);

    while( !oit.IsAtEnd() )
    {
        double value = 0;
        for (int i=0; i<m_WorldPositions.size(); i++)
        {
            mitk::Point3D c = m_WorldPositions.at(i);
            IndexType idx = oit.GetIndex();
            itk::Point<double, 3> vertex;
            outImage->TransformIndexToPhysicalPoint(idx, vertex);
            double dist = c.EuclideanDistanceTo(vertex);
            value += m_Heights.at(i)*exp(-dist*dist/(2*m_Variances.at(i)));
        }
        oit.Set(value);
        ++oit;
    }

    MITK_INFO << "Thread " << threadId << "finished processing";
}

}
