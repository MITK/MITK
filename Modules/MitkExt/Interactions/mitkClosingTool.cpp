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
  mitk::Image::Pointer new_image = mitk::Image::New();

  switch(m_StructElement)
  {
    case(BALL):

      m_Ball.SetRadius( m_Radius);
      m_Ball.CreateStructuringElement();

      m_BallClosingFilter = BallClosingFilterType::New();
      m_BallClosingFilter->SetKernel(m_Ball);
      m_BallClosingFilter->SetInput(itkImage);
      //m_BallClosingFilter->SetForegroundValue(1);
      //m_BallClosingFilter->SetBackgroundValue(0);
      m_BallClosingFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_BallClosingFilter->GetOutput(), new_image);
      break;

    case(CROSS):

      m_Cross.SetRadius(m_Radius);
      m_Cross.CreateStructuringElement();

      m_CrossClosingFilter = CrossClosingFilterType::New();
      m_CrossClosingFilter->SetKernel(m_Cross);
      m_CrossClosingFilter->SetInput(itkImage);
      //m_CrossClosingFilter->SetForegroundValue(1);
      //m_CrossClosingFilter->SetBackgroundValue(0);
      m_CrossClosingFilter->SetSafeBorder(true);
      m_CrossClosingFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_CrossClosingFilter->GetOutput(), new_image);
      break;

    default:
      break;
  }

  return new_image;
}
