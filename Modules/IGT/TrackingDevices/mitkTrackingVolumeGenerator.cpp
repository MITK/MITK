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

#include "mitkTrackingVolumeGenerator.h"
#include "mitkSTLFileReader.h"
#include "mitkStandardFileLocations.h"
#include "mitkConfig.h"
#include <vtkCubeSource.h>
#include <mitkTrackingTypes.h>
#include <mitkTrackingDevice.h>
#include <mitkVirtualTrackingDevice.h>
#include <vtkSmartPointer.h>
#include <mitkSurface.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <mitkIOUtil.h>



mitk::TrackingVolumeGenerator::TrackingVolumeGenerator()
{
  m_Data = mitk::DeviceDataUnspecified;
}


void mitk::TrackingVolumeGenerator::SetTrackingDevice (mitk::TrackingDevice::Pointer tracker)
{
  this->m_Data = mitk::GetFirstCompatibleDeviceDataForLine(tracker->GetType());
}

void mitk::TrackingVolumeGenerator::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();  //the surface wich represents the tracking volume

  std::string filepath = ""; // Full path to file (wil be resolved later)
  std::string filename = this->m_Data.VolumeModelLocation; // Name of the file or possibly a magic String, e.g. "cube"

  MITK_INFO << "volume: " << filename;

  // See if filename matches a magic string.
  if (filename.compare("cube") == 0){
    vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    double bounds[6];
    bounds[0] = bounds[2] = bounds[4] = -400.0;  // initialize bounds to -400 ... +400 cube. This is the default value of the
    bounds[1] = bounds[3] = bounds[5] =  400.0;  // virtual tracking device, but it can be changed. In that case,
    // the tracking volume polydata has to be updated manually
    cubeSource->SetBounds(bounds);
    cubeSource->Update();

    output->SetVtkPolyData(cubeSource->GetOutput()); //set the vtkCubeSource as polyData of the surface
    return;
  }
  if (filename.compare("") == 0) // empty String means no model, return empty output
  {
    // initialize with empty poly data (otherwise old surfaces may be returned) => so an empty surface is returned
    vtkPolyData *emptyPolyData = vtkPolyData::New();
    output->SetVtkPolyData(emptyPolyData);
    emptyPolyData->Delete();
    return;
  }

  // from here on, we assume that filename contains an actual filename and not a magic string

  us::Module* module = us::GetModuleContext()->GetModule();

  us::ModuleResource moduleResource = module->GetResource(filename);

  // Create ResourceStream from Resource
  us::ModuleResourceStream resStream(moduleResource,std::ios_base::binary);

  ofstream tmpOutputStream;
  std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpOutputStream);
  tmpOutputStream << resStream.rdbuf();
  tmpOutputStream.close();


  //filepath = mitk::StandardFileLocations::GetInstance()->FindFile(filename.c_str());

  if (tmpFilePath.empty())
  {
    MITK_ERROR << ("Volume Generator could not find the specified file " + filename);
    return;
  }

  mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
  stlReader->SetFileName( tmpFilePath.c_str() );
  stlReader->Update();

  std::remove(tmpFilePath.c_str());

  if ( stlReader->GetOutput() == NULL)
  {
    MITK_ERROR << "Error while reading file";
    return ;
  }
  output->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData());//set the visible trackingvolume
}

void mitk::TrackingVolumeGenerator::SetTrackingDeviceType(mitk::TrackingDeviceType deviceType)
{
  m_Data = mitk::GetFirstCompatibleDeviceDataForLine(deviceType);
}

mitk::TrackingDeviceType mitk::TrackingVolumeGenerator::GetTrackingDeviceType() const
{
  return m_Data.Line;
}

void mitk::TrackingVolumeGenerator::SetTrackingDeviceData(mitk::TrackingDeviceData deviceData)
{
  m_Data= deviceData;
}

mitk::TrackingDeviceData mitk::TrackingVolumeGenerator::GetTrackingDeviceData() const
{
  return m_Data;
}
