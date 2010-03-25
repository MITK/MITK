/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkTrackingVolume.h"
#include "mitkSTLFileReader.h"
#include "mitkStandardFileLocations.h"
#include "mitkConfig.h"
#include <vtkCubeSource.h>


mitk::TrackingVolume::TrackingVolume()
{
  m_TrackingDeviceType = mitk::TrackingSystemNotSpecified;

  //####### initialize file locations for the volume-STL-files #########
  std::string m_VolumeDir = MITK_ROOT;
  m_VolumeDir += "Modules/IGT/IGTTrackingDevices/TrackingVolumeData";
  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_VolumeDir.c_str(), false );
  //####################################################################

}

/* TODO: implementation of method
bool mitk::TrackingVolume::IsInside(mitk::Point3D itkNotUsed(punkt))
{
//NOT IMPLEMENTED YET!
return false;
}
*/

void mitk::TrackingVolume::SetVolumeManually(vtkPolyData* volume)
{
  this->SetVtkPolyData(volume);
}

bool mitk::TrackingVolume::SetTrackingDeviceType(TrackingDeviceType type)
{
  // get filename / perform custom initiation
  std::string filename = "";

  switch(type)
  {
  case mitk::ClaronMicron:
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronMicron.stl");
    break;
  case mitk::IntuitiveDaVinci:
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("IntuitiveDaVinci.stl");
    break;
  case mitk::NDIAurora:
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIAurora.stl");
    break;
  case mitk::NDIPolaris:
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIPolaris.stl");
    break;
  case mitk::VirtualTracker:
    {
      vtkCubeSource* cs = vtkCubeSource::New();
      double bounds[6];
       bounds[0] = bounds[2] = bounds[4] = -400.0;  // initialize bounds to -400 ... +400 cube. This is the default value of the 
       bounds[1] = bounds[3] = bounds[5] =  400.0;  // virtual tracking device, but it can be changed. In that case, 
      // the tracking volume polydata has be updated manually
      cs->SetBounds(bounds);
      cs->GetOutput()->Update();
      this->SetVtkPolyData(cs->GetOutput());
      cs->Delete();
      return true;
    }
  default:
    return false;
  }

  if (filename.empty())
    return false;

  mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
  try
  {
    stlReader->SetFileName( filename.c_str() );
    stlReader->Update();
  }
  catch (...)
  {
  	return false;
  }
  if ( stlReader->GetOutput() == NULL )
    return false;

  this->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData());
  stlReader = NULL;
  return true;  
}
