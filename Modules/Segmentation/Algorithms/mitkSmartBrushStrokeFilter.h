#pragma once

#include <itkImage.h>
#include <itkInPlaceImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <MitkSegmentationExports.h>

namespace mitk {

template<typename TImageType>
class SmartBrushStrokeFilter : public itk::InPlaceImageFilter<TImageType, itk::Image<float, 3>>
{
public:
  typedef SmartBrushStrokeFilter Self;
  typedef itk::InPlaceImageFilter<TImageType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  itkNewMacro(SmartBrushStrokeFilter);
  itkTypeMacro(SmartBrushStrokeFilter, InPlaceImageFilter);

  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  void SetRadius(int value);
  void SetTargetIntensity(float value);
  void SetCenter(mitk::Point3D value);
  void SetDirection(int value);
  void SetOriginalImage(itk::Image<float, 3>::Pointer value);
  void SetSensitivity(float value);
  void SetImageSpacing(itk::Vector<float, 3> spacing);

protected:
  static const float BETA_MIN;
  static const float BETA_MAX;
  static const int K_MIN = 1;
  static const int K_MAX = 5;

  SmartBrushStrokeFilter();
  virtual ~SmartBrushStrokeFilter();

  virtual void ThreadedGenerateData(const OutputImageRegionType&, itk::ThreadIdType);
  void BeforeThreadedGenerateData(void) ITK_OVERRIDE;

  double Distance(itk::Image<float, 3>::IndexType a, itk::Image<float, 3>::IndexType b);
  float CalcChange(itk::Image<float, 3>::IndexType x, float xValue);
  inline float FabsWithNaN(float a, float b);

  SmartBrushStrokeFilter(const SmartBrushStrokeFilter&);
  void operator=(const SmartBrushStrokeFilter&);

  int m_Radius;
  float m_TargetIntensity;
  itk::Image<float, 3>::IndexType m_Center;
  int m_Direction;
  itk::Image<float, 3>::Pointer m_OriginalImage;
  float m_Sensitivity;
  itk::Vector<float, 3> m_ImageSpacing;
};


template <typename TImageType> const float SmartBrushStrokeFilter<TImageType>::BETA_MIN = .01f;
template <typename TImageType> const float SmartBrushStrokeFilter<TImageType>::BETA_MAX = .1f;

template <typename TImageType>
double SmartBrushStrokeFilter<TImageType>::Distance(itk::Image<float, 3>::IndexType a, itk::Image<float, 3>::IndexType b)
{
  itk::Point<double, 3> p0;
  p0[0] = a[0] * m_ImageSpacing[0];
  p0[1] = a[1] * m_ImageSpacing[1];
  p0[2] = a[2] * m_ImageSpacing[2];

  itk::Point<double, 3> p1;
  p1[0] = b[0] * m_ImageSpacing[0];
  p1[1] = b[1] * m_ImageSpacing[1];
  p1[2] = b[2] * m_ImageSpacing[2];

  return p1.EuclideanDistanceTo(p0);
}

// If one of values is NaN returns 1.f
// If both of values are NaN returns 0.f
// If none of values are NaN returns fabs
template <typename TImageType>
float SmartBrushStrokeFilter<TImageType>::FabsWithNaN(float a, float b)
{
  return (!isnan(a) && !isnan(b)) ? fabs(a - b) : (isnan(a) + isnan(b)) % 2;
}

template <typename TImageType>
float SmartBrushStrokeFilter<TImageType>::CalcChange(itk::Image<float, 3>::IndexType x, float xValue)
{
  return BETA_MAX * pow(1 - FabsWithNaN(m_TargetIntensity, xValue), K_MIN + (K_MAX - K_MIN) * m_Sensitivity)
    * std::max((m_Radius - Distance(x, m_Center)) / m_Radius, 0.);
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::BeforeThreadedGenerateData()
{
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::ThreadedGenerateData(const OutputImageRegionType& region, itk::ThreadIdType threadId)
{
  typename TImageType::ConstPointer input = this->GetInput();
  typename itk::Image<float, 3>::Pointer output = this->GetOutput();

  itk::ImageRegionIteratorWithIndex<itk::Image<float, 3>> it(output, region);
  it.GoToBegin();

  while (!it.IsAtEnd()) {
    auto index = it.GetIndex();
    if (Distance(index, m_Center) > m_Radius) {
      it.Set(input->GetPixel(index));
      ++it;
      continue;
    }

    float change = CalcChange(index, m_OriginalImage->GetPixel(index));
    it.Set((1 - change) * input->GetPixel(index) + change * m_Direction);

    ++it;
  }
}

template <typename TImageType>
SmartBrushStrokeFilter<TImageType>::SmartBrushStrokeFilter()
{
}

template <typename TImageType>
SmartBrushStrokeFilter<TImageType>::~SmartBrushStrokeFilter()
{
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetRadius(int value)
{
  m_Radius = value;
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetTargetIntensity(float value)
{
  m_TargetIntensity = value;
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetCenter(mitk::Point3D value)
{
  m_Center[0] = value[0];
  m_Center[1] = value[1];
  m_Center[2] = value[2];
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetDirection(int value)
{
  m_Direction = value;
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetOriginalImage(itk::Image<float, 3>::Pointer value)
{
  m_OriginalImage = value;
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetSensitivity(float value)
{
  m_Sensitivity = value;
}

template <typename TImageType>
void SmartBrushStrokeFilter<TImageType>::SetImageSpacing(itk::Vector<float, 3> spacing)
{
  m_ImageSpacing = spacing;
}

}
