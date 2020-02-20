/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef _itkMITKScalarImageToHistogramGenerator_txx
#define _itkMITKScalarImageToHistogramGenerator_txx

#include "itkDefaultConvertPixelTraits.h"
#include "itkMITKScalarImageToHistogramGenerator.h"

namespace itk
{
  namespace Statistics
  {
    template <class TImage, class TMeasurementType>
    MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::MITKScalarImageToHistogramGenerator()
    {
      m_ImageToListAdaptor = AdaptorType::New();
      m_HistogramGenerator = GeneratorType::New();
      m_HistogramGenerator->SetInput(m_ImageToListAdaptor);
    }

    template <class TImage, class TMeasurementType>
    void MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::SetInput(const ImageType *image)
    {
      m_ImageToListAdaptor->SetImage(image);
    }

    template <class TImage, class TMeasurementType>
    const typename MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::HistogramType *
      MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::GetOutput() const
    {
      return m_HistogramGenerator->GetOutput();
    }

    template <class TImage, class TMeasurementType>
    void MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::Compute()
    {
      m_HistogramGenerator->Update();
    }

    template <class TImage, class TMeasurementType>
    void MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::SetNumberOfBins(unsigned int numberOfBins)
    {
      typename HistogramType::SizeType size;
      size.SetSize(itk::DefaultConvertPixelTraits<typename TImage::PixelType>::GetNumberOfComponents());
      size.Fill(numberOfBins);
      m_HistogramGenerator->SetHistogramSize(size);
    }

    template <class TImage, class TMeasurementType>
    void MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::SetMarginalScale(double marginalScale)
    {
      m_HistogramGenerator->SetMarginalScale(marginalScale);
    }

    template <class TImage, class TMeasurementType>
    void MITKScalarImageToHistogramGenerator<TImage, TMeasurementType>::PrintSelf(std::ostream &os, Indent indent) const
    {
      Superclass::PrintSelf(os, indent);
      os << "ImageToListSample adaptor = " << m_ImageToListAdaptor << std::endl;
      os << "HistogramGenerator = " << m_HistogramGenerator << std::endl;
    }

  } // end of namespace Statistics
} // end of namespace itk

#endif
