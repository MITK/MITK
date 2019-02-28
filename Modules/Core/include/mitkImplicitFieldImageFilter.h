#pragma once

#include <itkInPlaceImageFilter.h>
#include <vtkImplicitFunction.h>
#include <vtkPolyData.h>
#include <vtkImplicitPolyDataDistance.h>

#include <itkPolyLineParametricPath.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMedianImageFilter.h>

namespace mitk
{
template <typename TImageType>
class ImplicitFieldImageFilter : public itk::InPlaceImageFilter < TImageType, TImageType >
{
public:
  enum ResectionRegionType {
    INSIDE,
    OUTSIDE,
  };

  /** Standard class typedefs. */
  typedef ImplicitFieldImageFilter Self;
  typedef itk::InPlaceImageFilter< TImageType, TImageType > Superclass;
  typedef itk::SmartPointer< Self > Pointer;

  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
  typedef typename TImageType::RegionType ImageRegionType;
  typedef typename TImageType::PixelType ImagePixelType;
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImplicitFieldImageFilter, InPlaceImageFilter);

  void SetRegion(ImageRegionType region);
  void SetImplicitFunction(vtkImplicitFunction* f);
  void SetRegionType(ResectionRegionType type);

protected:
  ImplicitFieldImageFilter();
  virtual ~ImplicitFieldImageFilter();

  virtual void ThreadedGenerateData(const OutputImageRegionType &, itk::ThreadIdType);
  void ResectionFunction(const OutputImageRegionType &, itk::ThreadIdType);
  void BeforeThreadedGenerateData(void) ITK_OVERRIDE;

private:
  ImageRegionType m_Region;
  vtkSmartPointer<vtkImplicitFunction> m_ImplFunc;
  ResectionRegionType m_RegionType;
  ImagePixelType m_MinimumPixelValue;

  ImplicitFieldImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented
};

}
#include "mitkImplicitFieldImageFilter.hxx"

