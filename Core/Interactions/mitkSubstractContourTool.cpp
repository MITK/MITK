/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkSubstractContourTool.h"

#include "mitkSubstractContourTool.xpm"

mitk::SubstractContourTool::SubstractContourTool()
:ContourTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::SubstractContourTool::~SubstractContourTool()
{
}

const char** mitk::SubstractContourTool::GetXPM() const
{
  return mitkSubstractContourTool_xpm;
}

const char* mitk::SubstractContourTool::GetName() const
{
  return "Subtract";
}

