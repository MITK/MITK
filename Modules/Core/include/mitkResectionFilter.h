#pragma once
#include <limits>

#include <itkInPlaceImageFilter.h>
#include <itkImage.h>
#include <itkLogger.h>
#include <itkProgressAccumulator.h>

#include <vtkPoints.h>

#include <mitkImplicitPolyDataSweepDistance.h>
#include <mitkImplicitFieldImageFilter.h>

#include <MitkCoreExports.h>

namespace mitk
{
template <typename TImageType>
class ResectionFilter : public itk::InPlaceImageFilter < TImageType, TImageType >
  {
public:
  enum ResectionRegionType {
    INSIDE,
    OUTSIDE,
  };

  typedef ResectionFilter Self;
  typedef itk::InPlaceImageFilter < TImageType, TImageType > Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  typedef typename TImageType::PixelType ImagePixelType;

  itkNewMacro(Self);
  itkTypeMacro(ResectionFilter, itk::InPlaceImageFilter);

  itkSetObjectMacro(Logger, itk::Logger);

  itkSetObjectMacro(InputPoints, vtkPoints);

  void setRegionType(ResectionRegionType type);
  void setViewportMatrix(vtkMatrix4x4* matrix);

protected:
  ResectionFilter();
   ~ResectionFilter()ITK_OVERRIDE;

  virtual void GenerateData() ITK_OVERRIDE;
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const override;
  bool setupIntersection();
  void computeIntersection();

  // Logging
  itk::Logger::Pointer m_Logger;
  std::ostringstream m_Message;

  vtkSmartPointer<vtkPoints> m_InputPoints;
  vtkSmartPointer<vtkMatrix4x4> m_ViewportMatrix;
  vtkSmartPointer<mitk::ImplicitPolyDataSweepDistance> m_ImplicitDistance;
  ResectionRegionType m_RegionType;

  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

private:
  ResectionFilter(const Self&);
  void operator=(const Self&);
};

}
#include "mitkResectionFilter.hxx"

