#ifndef itkMultiHistogramFilter_h
#define itkMultiHistogramFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
  template<typename TInputImageType, typename TOuputImageType >
  class MultiHistogramFilter : public ImageToImageFilter< TInputImageType, TOuputImageType>
  {
    public:
      typedef MultiHistogramFilter                                    Self;
      typedef ImageToImageFilter< TInputImageType, TOuputImageType >  Superclass;
      typedef SmartPointer< Self >                                    Pointer;
      typedef typename TInputImageType::ConstPointer                       InputImagePointer;
      typedef typename TOuputImageType::Pointer                       OutputImagePointer;

      itkNewMacro (Self);
      itkTypeMacro(MultiHistogramFilter, ImageToImageFilter);

    protected:
      MultiHistogramFilter();
      ~MultiHistogramFilter(){};

      virtual void GenerateData();

      DataObject::Pointer MakeOutput(unsigned int idx);

      void CreateOutputImage(InputImagePointer input, OutputImagePointer output);

    private:
      MultiHistogramFilter(const Self &); // purposely not implemented
      void operator=(const Self &); // purposely not implemented
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiHistogramFilter.cpp"
#endif

#endif // itkMultiHistogramFilter_h
