#include "mitkDilateTool.h"
#include "mitkImageCast.h"

namespace mitk{
  MITK_TOOL_MACRO(MitkExt_EXPORT, DilateTool, "Dilate Tool");
}

mitk::DilateTool::DilateTool()
{
}

mitk::DilateTool::~DilateTool()
{

}

const char* mitk::DilateTool::GetName() const
{
  return "Dilate Tool";
}

const char** mitk::DilateTool::GetXPM() const
{
  return mitkDilateTool_xpm;
}

mitk::Image::Pointer mitk::DilateTool::ApplyFilter(mitk::Image::Pointer image)
{
  SegmentationType::Pointer itkImage = SegmentationType::New();
  mitk::CastToItkImage(image, itkImage);
  m_Ball.SetRadius( m_Radius);
  m_Ball.CreateStructuringElement();
  m_BallDilateFilter = BallDilateFilterType::New();
  m_BallDilateFilter->SetKernel(m_Ball);
  m_BallDilateFilter->SetInput(itkImage);
  m_BallDilateFilter->SetDilateValue(1);
  m_BallDilateFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer new_image = mitk::Image::New();
  mitk::CastToMitkImage(m_BallDilateFilter->GetOutput(), new_image);
  return new_image;
}
