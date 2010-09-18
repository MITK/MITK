#include "mitkErodeTool.h"
#include "mitkImageCast.h"
#include "mitkRenderingManager.h"

namespace mitk{
  MITK_TOOL_MACRO(MitkExt_EXPORT, ErodeTool, "Erode Tool");
}

mitk::ErodeTool::ErodeTool()
{

}

mitk::ErodeTool::~ErodeTool()
{

}

const char* mitk::ErodeTool::GetName() const
{
  return "Erode Tool";
}

const char** mitk::ErodeTool::GetXPM() const
{
  return mitkErodeTool_xpm;
}

mitk::Image::Pointer mitk::ErodeTool::ApplyFilter(mitk::Image::Pointer image)
{
  SegmentationType::Pointer itkImage = SegmentationType::New();
  mitk::CastToItkImage(image, itkImage);
  m_Ball.SetRadius( m_Radius);
  m_Ball.CreateStructuringElement();
  m_BallErodeFilter = BallErodeFilterType::New();
  m_BallErodeFilter->SetKernel(m_Ball);
  m_BallErodeFilter->SetInput(itkImage);
  m_BallErodeFilter->SetErodeValue(1);
  m_BallErodeFilter->BoundaryToForegroundOn();
  m_BallErodeFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer new_image = mitk::Image::New();
  mitk::CastToMitkImage(m_BallErodeFilter->GetOutput(), new_image);
  return new_image;
}