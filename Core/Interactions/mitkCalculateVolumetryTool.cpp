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

#include "mitkCalculateVolumetryTool.h"

#include "mitkCalculateVolumetryTool.xpm"

#include "mitkVolumeCalculator.h"
#include "mitkProperties.h"
#include "mitkToolManager.h"

mitk::CalculateVolumetryTool::CalculateVolumetryTool()
{
}

mitk::CalculateVolumetryTool::~CalculateVolumetryTool()
{
}

const char** mitk::CalculateVolumetryTool::GetXPM() const
{
  return mitkCalculateVolumetryTool_xpm;
}

const char* mitk::CalculateVolumetryTool::GetName() const
{
  return "Volumetry";
}

std::string mitk::CalculateVolumetryTool::GetErrorMessage()
{
  return "Volume could not be calculated for these nodes:";
}
    
bool mitk::CalculateVolumetryTool::ProcessOneWorkingData( DataTreeNode* node )
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
    if (image.IsNull()) return false;

    if (image->GetDimension() == 4)
    {
      Tool::ErrorMessage("Volumetry only valid for timestep 0! Bug #1280");
    }

    VolumeCalculator::Pointer volumetryFilter = VolumeCalculator::New();
    volumetryFilter->SetImage( image );
    volumetryFilter->SetThreshold( 1 ); // comparison is >=
    try
    {
      volumetryFilter->ComputeVolume();

      float volumeInTimeStep0 = volumetryFilter->GetVolume();

      node->SetProperty( "volume", FloatProperty::New(volumeInTimeStep0) );
    }
    catch(...)
    {
      return false;
    }
  }

  return true;
}

void mitk::CalculateVolumetryTool::FinishProcessingAllData()
{
  Superclass::FinishProcessingAllData();
  m_ToolManager->NodePropertiesChanged();
}

