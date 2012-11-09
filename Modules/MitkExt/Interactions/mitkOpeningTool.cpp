/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
  mitk::Image::Pointer new_image = mitk::Image::New();

  switch(m_StructElement)
  {
    case(BALL):

      m_Ball.SetRadius( m_Radius);
      m_Ball.CreateStructuringElement();

      m_BallOpeningFilter = BallOpeningFilterType::New();
      m_BallOpeningFilter->SetKernel(m_Ball);
      m_BallOpeningFilter->SetInput(itkImage);
      //m_BallOpeningFilter->SetForegroundValue(1);
      //m_BallOpeningFilter->SetBackgroundValue(0);
      m_BallOpeningFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_BallOpeningFilter->GetOutput(), new_image);
      break;

    case(CROSS):

      m_Cross.SetRadius(m_Radius);
      m_Cross.CreateStructuringElement();

      m_CrossOpeningFilter = CrossOpeningFilterType::New();
      m_CrossOpeningFilter->SetKernel(m_Cross);
      m_CrossOpeningFilter->SetInput(itkImage);
      //m_CrossOpeningFilter->SetForegroundValue(1);
      //m_CrossOpeningFilter->SetBackgroundValue(0);
      m_CrossOpeningFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_CrossOpeningFilter->GetOutput(), new_image);
      break;

    default:
      break;
  }

  return new_image;
}
