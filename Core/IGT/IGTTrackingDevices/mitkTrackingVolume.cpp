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
/**
 * NOT IMPLEMENTED YET!
 * @return Returns true if the point "punkt" is inside the trackingvolume. Be sure that the coordinates of "punkt" are in the tracking coordination system!
 */
bool mitk::TrackingVolume::IsInside(mitk::Point3D itkNotUsed(punkt))
  {
  return false;
  }
/**
 * \brief Sets the trackingvolume by hand. Only use this method if your specific trackingvolume is not supported by this class.
 */
void mitk::TrackingVolume::SetManualVolume(vtkPolyData* manualVolume)
  {
  this->SetVtkPolyData(manualVolume);
  }

/**
 * \brief Sets the typ of the trackingdevice. The dimensions of the trackingvolume are updated directly.
 */
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
