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
      typedef typename TOuputImageType::RegionType                    OutputImageRegionType;

      itkNewMacro (Self);
      itkTypeMacro(MultiHistogramFilter, ImageToImageFilter);

      itkSetMacro(Delta, double);
      itkGetConstMacro(Delta, double);

      itkSetMacro(Offset, double);
      itkGetConstMacro(Offset, double);

      itkSetMacro(Bins, int);
      itkGetConstMacro(Bins, int);

      itkSetMacro(Size, int);
      itkGetConstMacro(Size, int);

      itkSetMacro(UseImageIntensityRange, bool);
      itkGetConstMacro(UseImageIntensityRange, bool);

    protected:
      MultiHistogramFilter();
      ~MultiHistogramFilter(){};

      virtual void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId) override;
      virtual void BeforeThreadedGenerateData(void) override;


      using itk::ProcessObject::MakeOutput;
      itk::ProcessObject::DataObjectPointer MakeOutput(itk::ProcessObject::DataObjectPointerArraySizeType /*idx*/) override;

      void CreateOutputImage(InputImagePointer input, OutputImagePointer output);

    private:
      MultiHistogramFilter(const Self &); // purposely not implemented
      void operator=(const Self &); // purposely not implemented

      double m_Delta;
      double m_Offset;
      int m_Bins;
      int m_Size;
      bool m_UseImageIntensityRange;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiHistogramFilter.cpp"
#endif

#endif // itkMultiHistogramFilter_h
