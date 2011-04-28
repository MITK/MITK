#include "mitkTrackingVolumeGenerator.h"
#include "mitkSTLFileReader.h";
#include "mitkStandardFileLocations.h";
#include "mitkConfig.h"
#include <vtkCubeSource.h>

mitk::TrackingVolumeGenerator::TrackingVolumeGenerator()
{
    std::string m_VolumeDir = MITK_ROOT;
    m_VolumeDir += "Modules/IGT/IGTTrackingDevices/TrackingVolumeData";
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_VolumeDir.c_str(), false );
}

bool mitk::TrackingVolumeGenerator::SetTrackingDevice(mitk::TrackingDevice::Pointer tracker)
{
    std::cout << "es geht"<<std::endl;
}

