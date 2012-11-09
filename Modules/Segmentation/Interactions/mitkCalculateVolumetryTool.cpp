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

#include "mitkCalculateVolumetryTool.h"

#include "mitkCalculateVolumetryTool.xpm"

#include "mitkVolumeCalculator.h"
#include "mitkProperties.h"
#include "mitkToolManager.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, CalculateVolumetryTool, "Volumetry tool");
}

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

bool mitk::CalculateVolumetryTool::ProcessOneWorkingData( DataNode* node )
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

