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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef _itkMITKScalarImageToHistogramGenerator_txx
#define _itkMITKScalarImageToHistogramGenerator_txx

#include "itkMITKScalarImageToHistogramGenerator.h"


namespace itk {
namespace Statistics {


template < class TImage, class TMeasurementType >
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::MITKScalarImageToHistogramGenerator()
{
  m_ImageToListAdaptor = AdaptorType::New();
  m_HistogramGenerator = GeneratorType::New();
  m_HistogramGenerator->SetListSample( m_ImageToListAdaptor );
}



template < class TImage, class TMeasurementType >
void
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::SetInput( const ImageType * image )
{
  m_ImageToListAdaptor->SetImage( image );
}


template < class TImage, class TMeasurementType >
const typename MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >::HistogramType *
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::GetOutput() const
{
  return m_HistogramGenerator->GetOutput();
}



template < class TImage, class TMeasurementType >
void
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::Compute()
{
  m_HistogramGenerator->Update();
}



template < class TImage, class TMeasurementType >
void
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::SetNumberOfBins( unsigned int numberOfBins )
{
  typename HistogramType::SizeType size;
  size.Fill( numberOfBins );
  m_HistogramGenerator->SetNumberOfBins( size );
}



template < class TImage, class TMeasurementType >
void
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::SetMarginalScale( double marginalScale )
{
  m_HistogramGenerator->SetMarginalScale( marginalScale );
}




template < class TImage, class TMeasurementType >
void
MITKScalarImageToHistogramGenerator< TImage, TMeasurementType >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << "ImageToListSample adaptor = " << m_ImageToListAdaptor << std::endl;
  os << "HistogramGenerator = " << m_HistogramGenerator << std::endl;
}



} // end of namespace Statistics
} // end of namespace itk

#endif


