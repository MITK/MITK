#include "mitkClosingTool.h"
#include "mitkImageCast.h"
#include "mitkRenderingManager.h"

namespace mitk{
  MITK_TOOL_MACRO(MitkExt_EXPORT, ClosingTool, "Closing Tool");
}

mitk::ClosingTool::ClosingTool()
{

}

mitk::ClosingTool::~ClosingTool()
{

}

const char* mitk::ClosingTool::GetName() const
{
  return "Closing Tool";
}

const char** mitk::ClosingTool::GetXPM() const
{
  return mitkClosingTool_xpm;
}

mitk::Image::Pointer mitk::ClosingTool::ApplyFilter(mitk::Image::Pointer image)
{
  SegmentationType::Pointer itkImage = SegmentationType::New();
  mitk::CastToItkImage(image, itkImage);
  m_Ball.SetRadius( m_Radius);
  m_Ball.CreateStructuringElement();
  m_BallClosingFilter = BallClosingFilterType::New();
  m_BallClosingFilter->SetKernel(m_Ball);
  m_BallClosingFilter->SetInput(itkImage);
  //m_BallClosingFilter->SetRadius(1);
  m_BallClosingFilter->SetForegroundValue(1);
  //m_BallClosingFilter->SetBackgroundValue(0);
  m_BallClosingFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer new_image = mitk::Image::New();
  mitk::CastToMitkImage(m_BallClosingFilter->GetOutput(), new_image);
  return new_image;
}