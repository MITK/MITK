/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkLocalStatisticFilter_h
#define itkLocalStatisticFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
  template<typename TInputImageType, typename TOuputImageType >
  class LocalStatisticFilter : public ImageToImageFilter< TInputImageType, TOuputImageType>
  {
    public:
      typedef LocalStatisticFilter                                    Self;
      typedef ImageToImageFilter< TInputImageType, TOuputImageType >  Superclass;
      typedef SmartPointer< Self >                                    Pointer;
      typedef typename TInputImageType::ConstPointer                       InputImagePointer;
      typedef typename TOuputImageType::Pointer                       OutputImagePointer;
      typedef typename TOuputImageType::RegionType                    OutputImageRegionType;

      itkNewMacro (Self);
      itkTypeMacro(LocalStatisticFilter, ImageToImageFilter);

      itkSetMacro(Size, int);
      itkGetConstMacro(Size, int);

    protected:
      LocalStatisticFilter();
      ~LocalStatisticFilter() override{};

      void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId) override;
      void BeforeThreadedGenerateData(void) override;


      using itk::ProcessObject::MakeOutput;
      itk::ProcessObject::DataObjectPointer MakeOutput(itk::ProcessObject::DataObjectPointerArraySizeType /*idx*/) override;

      void CreateOutputImage(InputImagePointer input, OutputImagePointer output);

    private:
      LocalStatisticFilter(const Self &); // purposely not implemented
      void operator=(const Self &); // purposely not implemented

      int m_Size;
      int m_Bins;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLocalStatisticFilter.hxx"
#endif

#endif // itkLocalStatisticFilter_h
