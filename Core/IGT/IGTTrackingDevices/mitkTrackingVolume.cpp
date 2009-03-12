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

mitk::TrackingVolume::TrackingVolume()
  {
  this->SetTrackingDeviceType(mitk::TrackingSystemNotSpecified);

  //####### initialize file locations for the volume-STL-files #########
  std::string m_VolumeDir = MITK_ROOT;
  m_VolumeDir += "Core/IGT/IGTTrackingDevices/TrackingVolumeData";
  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_VolumeDir.c_str(), false );
  //####################################################################

  }

bool mitk::TrackingVolume::IsInside(mitk::Point3D itkNotUsed(punkt))
  {
  //NOT IMPLEMENTED YET!
  return false;
  }

void mitk::TrackingVolume::SetManualVolume(vtkPolyData* manualVolume)
  {
  this->SetVtkPolyData(manualVolume);
  }

bool mitk::TrackingVolume::SetTrackingDeviceType(TrackingDeviceType type)
  {
  //Dateinamen Anhand des Trackingsystems bestimmen:
  std::string filename = "";
  if (type == mitk::ClaronMicron)
    {
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronMicron.stl");
    }
  else if (type == mitk::IntuitiveDaVinci)
    {
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("IntuitiveDaVinci.stl");
    }
  else if (type == mitk::NDIAurora)
    {
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIAurora.stl");
    }
  else if (type == mitk::NDIPolaris)
    {
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIPolaris.stl");
    }
  else if (type == mitk::TrackingSystemNotSpecified)
    {
    filename = mitk::StandardFileLocations::GetInstance()->FindFile("StandardVolume.stl");
    }

  //Die Surface aus der Datei einlesen:
  mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
  stlReader->SetFileName( filename.c_str() );
  stlReader->Update();
  if ( stlReader->GetOutput() != NULL )
    {
    this->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData());
    return true;
    }
  else
    {
    return false;
    }
  }