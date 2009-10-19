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

#include "mitkAutoCropTool.h"

#include "mitkAutoCropTool.xpm"

#include "mitkAutoCropImageFilter.h"

namespace mitk {
  MITK_TOOL_MACRO(MITKEXT_CORE_EXPORT, AutoCropImageFilter, "Crop tool");
}

mitk::AutoCropTool::AutoCropTool()
{
}

mitk::AutoCropTool::~AutoCropTool()
{
}

const char** mitk::AutoCropTool::GetXPM() const
{
  return mitkAutoCropTool_xpm;
}

const char* mitk::AutoCropTool::GetName() const
{
  return "Crop";
}

std::string mitk::AutoCropTool::GetErrorMessage()
{
  return "Cropping of these nodes failed:";
}
    
bool mitk::AutoCropTool::ProcessOneWorkingData( DataTreeNode* node )
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
    if (image.IsNull()) return false;
      
//     if (image->GetDimension() == 4)
//     {
//       Tool::ErrorMessage.Send("Cropping 3D+t segmentations is not implemented. Sorry. Bug #1281");
//       return false;
//     }

    AutoCropImageFilter::Pointer cropFilter = AutoCropImageFilter::New();
    cropFilter->SetInput( image );
    cropFilter->SetBackgroundValue( 0 );
    try
    {
      cropFilter->Update();

      image = cropFilter->GetOutput();
      if (image.IsNotNull())
      {
        node->SetData( image );
      }
      else
      {
        return false;
      }
    }
    catch(...)
    {
      return false;
    }
  }

  return true;
}

