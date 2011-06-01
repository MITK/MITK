/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2011-01-18 13:22:38 +0100 (Di, 18 Jan 2011) $
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

mitk::TrackingVolumeGenerator::TrackingVolumeGenerator()
{
    std::string volumeDir = MITK_ROOT;
    volumeDir += "Modules/IGT/IGTTrackingDevices/TrackingVolumeData"; //folder which contains the trackingdevices configs
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( volumeDir.c_str(), false ); //add this directory to StdFileLocations for the search

    m_TrackingDeviceType = mitk::TrackingSystemNotSpecified;
}


void mitk::TrackingVolumeGenerator::SetTrackingDevice (mitk::TrackingDevice::Pointer tracker)
{
    this->m_TrackingDeviceType = tracker->GetType();
}

void mitk::TrackingVolumeGenerator::GenerateData()
{
    mitk::Surface::Pointer output = this->GetOutput();//the surface wich represents the tracking volume

    std::string filename = "";

    switch(m_TrackingDeviceType)
    {
    case mitk::ClaronMicron:
        filename = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronMicron.stl");
        break;
    case mitk::IntuitiveDaVinci:
        filename = mitk::StandardFileLocations::GetInstance()->FindFile("IntuitiveDaVinci.stl");
        break;
    case mitk::NDIPolaris:
        filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIPolaris.stl");
        break;
    case mitk::NDIAurora:
        filename = mitk::StandardFileLocations::GetInstance()->FindFile("NDIAurora.stl");
        break;
    case mitk::TrackingSystemNotSpecified:
    case mitk::VirtualTracker:
        {
           vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
            double bounds[6];
            bounds[0] = bounds[2] = bounds[4] = -400.0;  // initialize bounds to -400 ... +400 cube. This is the default value of the
            bounds[1] = bounds[3] = bounds[5] =  400.0;  // virtual tracking device, but it can be changed. In that case,
            // the tracking volume polydata has be updated manually
            cubeSource->SetBounds(bounds);
            cubeSource->GetOutput()->Update();

            output->SetVtkPolyData(cubeSource->GetOutput()); //set the vtkCubeSource as polyData of the surface
            return;
        }
    default:
        {
            output->SetVtkPolyData(vtkPolyData::New()); //initialize with empty poly data (otherwise old surfaces may be returned) => so an empty surface is returned
            MITK_ERROR<< "No STL to given TrackingDevice found";
            return;
        }

    }

    if (filename.empty())
    {
        MITK_ERROR << "Filename is empty";
        return;
    }

    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    stlReader->SetFileName( filename.c_str() );
    stlReader->Update();

    if ( stlReader->GetOutput() == NULL)
    {
        MITK_ERROR << "Error while reading file";
        return ;
    }
    output->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData());//set the visible trackingvolume
}



