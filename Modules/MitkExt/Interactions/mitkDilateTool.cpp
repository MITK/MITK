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
  mitk::Image::Pointer new_image = mitk::Image::New();

  switch(m_StructElement)
  {
    case(BALL):

      m_Ball.SetRadius( m_Radius);
      m_Ball.CreateStructuringElement();

      m_BallDilateFilter = BallDilateFilterType::New();
      m_BallDilateFilter->SetKernel(m_Ball);
      m_BallDilateFilter->SetInput(itkImage);
      m_BallDilateFilter->SetDilateValue(1);
      m_BallDilateFilter->BoundaryToForegroundOn();
      m_BallDilateFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_BallDilateFilter->GetOutput(), new_image);
      break;

    case(CROSS):

      m_Cross.SetRadius(m_Radius);
      m_Cross.CreateStructuringElement();

      m_CrossDilateFilter = CrossDilateFilterType::New();
      m_CrossDilateFilter->SetKernel(m_Cross);
      m_CrossDilateFilter->SetInput(itkImage);
      m_CrossDilateFilter->SetDilateValue(1);
      m_CrossDilateFilter->BoundaryToForegroundOn();
      m_CrossDilateFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(m_CrossDilateFilter->GetOutput(), new_image);
      break;

    default:
      break;
  }

  return new_image;
}
