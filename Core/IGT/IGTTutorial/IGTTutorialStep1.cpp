/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkRandomTrackingDevice.h>
#include <mitkInternalTrackingTool.h>

#include <mitkNavigationData.h>
#include <mitkTrackingDeviceSource.h>
#include "mitkNavigationDataDisplacementFilter.h"
#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>

#include <itksys/SystemTools.hxx>

//##Documentation
//## \brief A small console tutorial about MITK-IGT
int main(int argc, char* argv[])
{


  //*************************************************************************
  // What we will do...
  //*************************************************************************
  //In this tutorial we build up a small navigation pipeline with a virtual tracking device
  //which produce random positions and orientation so no additional hardware is required.
  //The source of the pipeline is a TrackingDeviceSource object. This we connect to a simple
  //filter which just displaces the positions with an offset. After that we use a recorder
  //to store this new positions and other information to disc in a XML file. After that we use
  //another source (NavigationDataPlayer) to replay the recorded data.
  

  //*************************************************************************
  // Part I: Basic initialization of the source and tracking device
  //*************************************************************************
  //First of all create a tracking device object and two tools for this "device".

  //Here we take the RandomTrackingDevice. This is not a real tracking device it just delivers random 
  //positions and orientations. You can use other/real tracking devices if you replace the following 
  //code with different tracking devices, e.g. mitk::NDITrackingDevice. The tools represent the
  //sensors of the tracking device. The TrackingDevice fills the tools with data.
  std::cout << "Generating TrackingDevice ..." << std::endl;

  mitk::RandomTrackingDevice::Pointer tracker = mitk::RandomTrackingDevice::New();
  tracker->AddTool("tool1");
  tracker->AddTool("tool2");

  //The tracking device object is used for the physical connection to the device. To use the
  //data inside of our tracking pipeline we need a source. This source encapsulate the tracking device
  //and provides objects of the type mitk::NavigationData as output. The NavigationData objects stores 
  //position, orientation, if the data is valid or not and special error informations in a covariance 
  //matrix.
  //
  //Typically the start of our pipeline is a TrackingDeviceSource. To work correct we have to set a 
  //TrackingDevice object. Attention you have to set the tools before you set the whole TrackingDevice
  //object to the TrackingDeviceSource because the source need to know how many outputs should be 
  //generated.

  std::cout << "Generating Source ..." << std::endl;

  mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
  source->SetTrackingDevice(tracker); //here we set the device for the pipeline source

  source->Connect();        //here we connect to the tracking system
                            //Note we do not call this on the TrackingDevice object
  source->StartTracking();  //start the tracking 
                            //Now the source generates outputs.


  //*************************************************************************
  // Part II: Create a NavigationDataToNavigationDataFilter
  //*************************************************************************

  //The next thing we do is using a NavigationDataToNavigationDataFilter. One of these filter is the 
  //very simple NavigationDataDisplacementFilter. This filter just changes the positions of the input 
  //NavigationData objects with an offset for each direction (X,Y,Z). The input of this filter is the 
  //source and the output of this filter is the "displaced" input.

  std::cout << "Generating DisplacementFilter ..." << std::endl;

  mitk::NavigationDataDisplacementFilter::Pointer displacer = mitk::NavigationDataDisplacementFilter::New();
  mitk::Vector3D offset;
  mitk::FillVector3D(offset, 10.0, 100.0, 1.0); //initialize the offset
  displacer->SetOffset(offset); //now set the offset in the NavigationDataDisplacementFilter object

  //now every output of the source object is connected to the displacer object
  for (int i = 0; i < source->GetNumberOfOutputs(); i++)
  {
    displacer->SetInput(i, source->GetOutput(i));  //here we connect to the displacement filter 
  }


  //*************************************************************************
  // Part III: Record the data with the NavigationDataRecorder
  //*************************************************************************

  //The next part of our pipeline is the recorder. The recorder needs a filename. Otherwise the output
  //is redirected to the console. The input of the recorder is the output of the displacement filter
  //and the output is a XML file with the name "Test Output-0.xml".

  std::cout << "Start Recording ..." << std::endl;

  //we need the stringstream for building up our filename
  std::stringstream filename;

  //the .xml extension and an counter is added automatically
  filename << itksys::SystemTools::GetCurrentWorkingDirectory() << "/Test Output";

  std::cout << "Record to file: " << filename.str() << "-0.xml ..." << std::endl;

  mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
  recorder->SetFileName(filename.str());

  //now every output of the displacer object is connected to the recorder object
  for (int i = 0; i < displacer->GetNumberOfOutputs(); i++)
  {
    recorder->AddNavigationData(displacer->GetOutput(i));  // here we connect to the recorder
  }

  recorder->StartRecording(); //after finishing the settings you can start the recording mechanism 
                              //now every update of the recorder stores one line into the file for 
                              //each added NavigationData


  for (unsigned int x = 0; x<100; x++) //write 100 datasets
  {
    recorder->Update(); //the update causes one line in the XML file for every tool
                        //in this case two lines
    itksys::SystemTools::Delay(100);         //sleep a little
  }
  recorder->StopRecording(); //to get proper XML files you should stop recording
                             //if your application crashes during recording no data 
                             //will be lost it is all stored to disc


  //*************************************************************************
  // Part IV: Play the data with the NavigationDataPlayer
  //*************************************************************************

  //The recording is finished now so now we can play the data. The NavigationDataPlayer is similar 
  //to the TrackingDevice source. It also derives from NavigationDataSource. So you can use a player 
  //instead of a TrackingDeviceSource. The input of this player is the filename and the output are 
  //NavigationData object.

  filename << "-0.xml";
  std::cout << "Start playing from file: " << filename.str() << " ..." << std::endl;


  mitk::NavigationDataPlayer::Pointer player = mitk::NavigationDataPlayer::New();
  //this is first part of the file name the .xml extension and an counter is added automatically
  player->SetFileName(filename.str()); 
  player->StartPlaying(); //this starts the player
                          //From now on the player provides NavigationDatas in the order and 
                          //correct time as they were recorded
  
  //this connects the outputs of the player to the NavigationData objects
  mitk::NavigationData::Pointer nd = player->GetOutput(0); 
  mitk::NavigationData::Pointer nd2 = player->GetOutput(1); 
  for (unsigned int x=0; x<100; x++) 
  {
    if (nd.IsNotNull()) //check if the output is not null
    {
      //With this update the NavigationData object propagates through the pipeline to get a new value.
      //In this case we only have a source (NavigationDataPlayer).
      nd->Update(); 

      std::cout << x << ": 1:" << nd->GetPosition() << std::endl;
      std::cout << x << ": 2:" << nd2->GetPosition() << std::endl;
      std::cout << x << ": 1:" << nd->GetOrientation() << std::endl;
      std::cout << x << ": 2:" << nd2->GetOrientation() << std::endl;

      itksys::SystemTools::Delay(100); //sleep a little like in the recorder part
    }
  }
  player->StopPlaying(); //This stops the player
                         //With another call of StartPlaying the player will start again at the beginning of the file

  itksys::SystemTools::Delay(2000);
  std::cout << "finished" << std::endl;

}


