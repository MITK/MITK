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
      ~LocalStatisticFilter(){};

      virtual void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId);
      virtual void BeforeThreadedGenerateData(void);


      using itk::ProcessObject::MakeOutput;
      virtual itk::ProcessObject::DataObjectPointer MakeOutput(itk::ProcessObject::DataObjectPointerArraySizeType /*idx*/) override;

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
