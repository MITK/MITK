#include <mitkRandomTrackingDevice.h>
#include <mitkInternalTrackingTool.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataRecorder.h>



//##Documentation
//## @brief Show how MITK-IGT works
int main(int argc, char* argv[])
{


  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  //First of all create a tracking device objects
  //There are the NDI Poloaris/Aurora and the Micron tracking devices available
  //Writing own tracking devices is quiet simple have a look at the abstract class mitk::TrackingDevice 
  mitk::RandomTrackingDevice::Pointer tracker = mitk::RandomTrackingDevice::New();
  mitk::InternalTrackingTool::Pointer tool1 = mitk::InternalTrackingTool::New();
  mitk::InternalTrackingTool::Pointer tool2 = mitk::InternalTrackingTool::New();
  tracker->AddTool(tool1);
  tracker->AddTool(tool2);
  tracker->SetRefreshRate(0);


  mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
  source->SetTrackingDevice(tracker);


  mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
  recorder->SetOutputMode(mitk::NavigationDataRecorder::NormalFile);

  recorder->SetFileName("Test Output"); //this is first part of the file name the .xml extension and an counter is added automatically
  recorder->SetFilePath("C://"); //the path where the file is stored in

  recorder->AddNavigationData(source->GetOutput(0));
  recorder->AddNavigationData(source->GetOutput(1));

  recorder->StartRecording(); //after finishing the settings you can start the recording mechanism 
                              //now every update of the recorder stores one line into the file for each added NavigationData

  source->Connect();
  source->StartTracking();
  for (unsigned int x=0; x<100; x++) //write 100 datasets
  {
    source->GetOutput()->Update();
    recorder->Update();
  }
  recorder->StopRecording(); //to get proper XML files you should stop recording

  //recorder->StartRecording();
  //for (unsigned int x=0; x<100; x++) //write 50 datasets
  //{
  //  source->GetOutput()->Update();
  //  recorder->Update();
  //}
  //recorder->StopRecording();
  std::cout << "finished";
  //*************************************************************************
  // Part II: Create some data by reading a file
  //*************************************************************************



  //*************************************************************************
  // Part III: Put the data into the datastorage
  //*************************************************************************



  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************



  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************

}

/**
\example Step1.cpp
*/

