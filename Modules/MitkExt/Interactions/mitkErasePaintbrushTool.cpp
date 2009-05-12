/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkErasePaintbrushTool.h"

#include "mitkErasePaintbrushTool.xpm"

mitk::ErasePaintbrushTool::ErasePaintbrushTool()
:PaintbrushTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::ErasePaintbrushTool::~ErasePaintbrushTool()
{
}

const char** mitk::ErasePaintbrushTool::GetXPM() const
{
  return mitkErasePaintbrushTool_xpm;
}

const char* mitk::ErasePaintbrushTool::GetName() const
{
  return "Wipe";
}

