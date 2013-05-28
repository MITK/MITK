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
    m_Gradient.fill(0.0);
    m_Offset.fill(0.0);
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

    int szx = m_ImageRegion.GetSize(0);
    int szy = m_ImageRegion.GetSize(1);
    int szz = m_ImageRegion.GetSize(2);

    double max = szx*szx + szy*szy - szx*szy;

    while( !oit.IsAtEnd() )
    {
        double value = 0;
        IndexType idx = oit.GetIndex();
//        for (int i=0; i<3; i++)
//            value += std::exp(-(double)idx[i]/32.0)*m_Gradient[i] + m_Offset[i];

        //value += idx[0]*idx[0] + idx[1]*idx[1] - idx[0]*idx[1];

        for (int i=0; i<m_WorldPositions.size(); i++)
        {
            mitk::Point3D c = m_WorldPositions.at(i);
            itk::Point<double, 3> vertex;
            outImage->TransformIndexToPhysicalPoint(idx, vertex);
            double dist = c.EuclideanDistanceTo(vertex);

            c[0] -= vertex[0];
            c[1] -= vertex[1];
            c[2] -= vertex[2];

            double x = c[0];
            double y = c[1];
            double z = c[2];

            dist = fabs(dist) - m_Variances.at(i);
            if (dist>0)
                value += (2*z*z-y*y-x*x)/pow(x*x+y*y+z*z,5/2);


//            dist = fabs(dist) - m_Variances.at(i);
//            if (dist<0)
//                dist = 0;
//            value += std::exp(-dist/m_Variances.at(i))*m_Heights.at(i);

            // value += m_Heights.at(i)*exp(-dist*dist/(2*m_Variances.at(i)));
        }

        oit.Set(m_Gradient[0]*value/max);
        ++oit;
    }

    MITK_INFO << "Thread " << threadId << "finished processing";
}

}
