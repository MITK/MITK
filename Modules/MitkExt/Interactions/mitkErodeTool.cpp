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
  mitk::Image::Pointer new_image = mitk::Image::New();

  switch(m_StructElement)
  {
    case(BALL):

      m_Ball.SetRadius( m_Radius);
      m_Ball.CreateStructuringElement();

      m_BallErodeFilter = BallErodeFilterType::New();
      m_BallErodeFilter->SetKernel(m_Ball);
      m_BallErodeFilter->SetInput(itkImage);
      m_BallErodeFilter->SetErodeValue(1);
      m_BallErodeFilter->BoundaryToForegroundOn();
      m_BallErodeFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_BallErodeFilter->GetOutput(), new_image);
      break;

    case(CROSS):

      m_Cross.SetRadius(m_Radius);
      m_Cross.CreateStructuringElement();

      m_CrossErodeFilter = CrossErodeFilterType::New();
      m_CrossErodeFilter->SetKernel(m_Cross);
      m_CrossErodeFilter->SetInput(itkImage);
      m_CrossErodeFilter->SetErodeValue(1);
      m_CrossErodeFilter->BoundaryToForegroundOn();
      m_CrossErodeFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_CrossErodeFilter->GetOutput(), new_image);
      break;

    default:
      break;
  }

  return new_image;
}
