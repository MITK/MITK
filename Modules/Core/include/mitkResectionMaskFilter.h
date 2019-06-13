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
class ResectionMaskFilter : public itk::InPlaceImageFilter < TImageType, itk::Image<char, 3> >
  {
public:
  typedef ResectionMaskFilter Self;
  typedef itk::InPlaceImageFilter < TImageType, itk::Image<char, 3> > Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  typedef typename TImageType::PixelType ImagePixelType;

  itkNewMacro(Self);
  itkTypeMacro(ResectionMaskFilter, itk::InPlaceImageFilter);

  itkSetObjectMacro(Logger, itk::Logger);

  itkSetObjectMacro(InputPoints, vtkPoints);

  void setViewportMatrix(vtkMatrix4x4* matrix);

protected:
  ResectionMaskFilter();
   ~ResectionMaskFilter()ITK_OVERRIDE;

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

  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

private:
  ResectionMaskFilter(const Self&);
  void operator=(const Self&);
};

}
#include "mitkResectionMaskFilter.hxx"

