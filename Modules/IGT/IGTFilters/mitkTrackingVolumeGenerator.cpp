#include "mitkTrackingVolumeGenerator.h"
#include "mitkSTLFileReader.h"
#include "mitkStandardFileLocations.h"
#include "mitkConfig.h"
#include <vtkCubeSource.h>
#include <mitkTrackingTypes.h>
#include <mitkTrackingDevice.h>

mitk::TrackingVolumeGenerator::TrackingVolumeGenerator()
{
    std::string m_VolumeDir = MITK_ROOT;
    m_VolumeDir += "Modules/IGT/IGTTrackingDevices/TrackingVolumeData";
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_VolumeDir.c_str(), false );

    MITK_INFO << "Konstruktor geht";

}



mitk::Surface::Pointer mitk::TrackingVolumeGenerator::SetTrackingDevice(mitk::TrackingDevice::Pointer tracker)
{
    MITK_INFO << "es geht";
    TrackingDeviceType type = tracker->GetType();
    // get filename / perform custom initiation
    std::string filename = "";

    switch(type)
    {
    case mitk::ClaronMicron:
      filename = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronMicron.stl");
      MITK_INFO << "Claron";
      break;
    case mitk::IntuitiveDaVinci:
      filename = mitk::StandardFileLocations::GetInstance()->FindFile("IntuitiveDaVinci.stl");
      break;
    case mitk::NDIAurora:
      filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIAurora.stl");
      MITK_INFO << "Aurora";
      break;
    case mitk::NDIPolaris:
      filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIPolaris.stl");
      MITK_INFO << "Polaris";
      break;
    case mitk::VirtualTracker:
      {/*
        vtkCubeSource* cs = vtkCubeSource::New();
        double bounds[6];
         bounds[0] = bounds[2] = bounds[4] = -400.0;  // initialize bounds to -400 ... +400 cube. This is the default value of the
         bounds[1] = bounds[3] = bounds[5] =  400.0;  // virtual tracking device, but it can be changed. In that case,
        // the tracking volume polydata has be updated manually
        cs->SetBounds(bounds);
        cs->GetOutput()->Update();
        this->SetVtkPolyData(cs->GetOutput());
        cs->Delete();*/
        MITK_INFO << "Virtual Tracker !";
        return NULL;
      }
    default:
      return NULL;
    }

    if (filename.empty())
      return NULL;

    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
    {
      stlReader->SetFileName( filename.c_str() );
      stlReader->Update();
    }
    catch (...)
    {
          return NULL;
    }
    if ( stlReader->GetOutput() == NULL )
      return false;
    mitk::Surface::Pointer nerv = this->GetOutput ();


    nerv->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData());
    stlReader = NULL;

    MITK_INFO << "seems to work";

    return nerv;

}

