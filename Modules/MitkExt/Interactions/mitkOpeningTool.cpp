#include "mitkOpeningTool.h"
#include "mitkImageCast.h"
#include "mitkRenderingManager.h"

namespace mitk{
  MITK_TOOL_MACRO(MitkExt_EXPORT, OpeningTool, "Opening Tool");
}

mitk::OpeningTool::OpeningTool()
{

}

mitk::OpeningTool::~OpeningTool()
{

}

const char* mitk::OpeningTool::GetName() const
{
  return "Opening Tool";
}

const char** mitk::OpeningTool::GetXPM() const
{
  return mitkOpeningTool_xpm;
}

mitk::Image::Pointer mitk::OpeningTool::ApplyFilter(mitk::Image::Pointer image)
{
  SegmentationType::Pointer itkImage = SegmentationType::New();
  mitk::CastToItkImage(image, itkImage);
  m_Ball.SetRadius( m_Radius);
  m_Ball.CreateStructuringElement();
  m_BallOpeningFilter = BallOpeningFilterType::New();
  m_BallOpeningFilter->SetKernel(m_Ball);
  m_BallOpeningFilter->SetInput(itkImage);
  //m_BallOpeningFilter->SetRadius(1);
  m_BallOpeningFilter->SetForegroundValue(1);
  m_BallOpeningFilter->SetBackgroundValue(0);
  m_BallOpeningFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer new_image = mitk::Image::New();
  mitk::CastToMitkImage(m_BallOpeningFilter->GetOutput(), new_image);
  return new_image;
}