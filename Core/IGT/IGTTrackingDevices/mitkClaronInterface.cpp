#include <mitkClaronInterface.h>
#include <string>
#include <MTC.h>
#include <math.h>
#include <mitkVector.h>

mitk::ClaronInterface::ClaronInterface(std::string calibrationDir, std::string toolFilesDir)
{
  sprintf(this->calibrationDir, calibrationDir.c_str());
  sprintf(this->markerDir,toolFilesDir.c_str());
}


bool mitk::ClaronInterface::StartTracking()
{
  isTracking = false;
  MTC( Cameras_AttachAvailableCameras(calibrationDir) ); //Connect to camera
  if (Cameras_Count() < 1)
  {
    printf("No camera found!\n");
    return false;
  }

  try
  {
    //Step 1: initialize cameras
    MTC(Cameras_HistogramEqualizeImagesSet(true)); //set the histogram equalizing
    MTC( Cameras_ItemGet(0, &CurrCamera) ); //Obtain a handle to the first/only camera in the array

    //Step 2: Load the marker templates
    MTC( Markers_LoadTemplates(markerDir) ); //Path to directory where the marker templates are
    printf("Loaded %d marker templates\n",Markers_TemplatesCount());

    //Step 3: Wait a 20 frames
    for (int i=0; i<20; i++)//the first 20 frames are auto-adjustment frames, we ignore them
    {
      MTC( Cameras_GrabFrame(NULL) ); //Grab a frame (all cameras together)
      MTC( Markers_ProcessFrame(NULL) ); //Proces the frame(s) to obtain measurements
    }

    //Step 4: Initialize IdentifiedMarkers and PoseXf
    IdentifiedMarkers = Collection_New();
    PoseXf = Xform3D_New();
    
    //now we are tracking...

    /* MTHome is not in use. The following code has to be activated if you want to use MTHome!
    //Initialize MTHome
    if ( getMTHome (MTHome, sizeof(MTHome)) < 0 )
    {
    // No Environment
    printf("MTHome environment variable is not set!\n");
    }*/
  }
  catch(...)
  {
    Cameras_Detach();
    printf(" Error while connecting MicronTracker!\n -------------------------------");
    return false;
  }

  isTracking = true;
  return true;
}

bool mitk::ClaronInterface::StopTracking()
{
  if (isTracking)
  {
    //free up the resources
    Collection_Free(IdentifiedMarkers);
    Xform3D_Free(PoseXf);

    //stop the camera
    Cameras_Detach();
    
    //now tracking is stopped
    isTracking = false;
    return true;
  }
  else
  {
    return false;
  }
}

std::vector<mitk::claronToolHandle> mitk::ClaronInterface::GetAllActiveTools()
{
  //Set returnvalue
  std::vector<claronToolHandle> returnValue;

  //Here, MTC internally maintains the measurement results.
  //Those results can be accessed until the next call to Markers_ProcessFrame, when they
  //are updated to reflect the next frame's content.
  //First, we will obtain the collection of the markers that were identified.
  MTC( Markers_IdentifiedMarkersGet(NULL, IdentifiedMarkers) );
  
  //Now we iterate on the identified markers and add them to the returnvalue
  for (int j=1; j<=Collection_Count(IdentifiedMarkers); j++)
  {
    // Obtain the marker's handle, and use it to obtain the pose in the current camera's space
    // using our Xform3D object, PoseXf.
    mtHandle Marker = Collection_Int(IdentifiedMarkers, j);
    returnValue.push_back(Marker);
  }
  return returnValue;
}

void mitk::ClaronInterface::GrabFrame()
{
  MTC( Cameras_GrabFrame(NULL) ); //Grab a frame
  MTC( Markers_ProcessFrame(NULL) ); //Process the frame(s)
}

std::vector<double> mitk::ClaronInterface::GetTipPosition(mitk::claronToolHandle c)
{
  std::vector<double> returnValue;
  double	Position[3];
  mtHandle t2m = Xform3D_New(); // tooltip to marker xform handle
  mtHandle t2c = Xform3D_New(); // tooltip to camera xform handle
  mtHandle m2c = Xform3D_New(); // marker to camera xform handle

  //Get m2c
  MTC( Marker_Marker2CameraXfGet (c, CurrCamera, m2c, &IdentifyingCamera) );
  //Get t2m
  MTC( Marker_Tooltip2MarkerXfGet (c,  t2m ));
  //Transform both to t2c
  MTC(Xform3D_Concatenate(t2m,m2c,t2c));

  //Get position
  MTC( Xform3D_ShiftGet(t2c, Position) );

  // Here we have to negate the X- and Y-coordinates because of a bug of the
  // MTC-library.
  returnValue.push_back(-Position[0]);
  returnValue.push_back(-Position[1]);
  returnValue.push_back(Position[2]);

  return returnValue;
}

std::vector<double> mitk::ClaronInterface::GetPosition(claronToolHandle c)
{
  std::vector<double> returnValue;
  double	Position[3];
  MTC( Marker_Marker2CameraXfGet (c, CurrCamera, PoseXf, &IdentifyingCamera) );
  MTC( Xform3D_ShiftGet(PoseXf, Position) );

  // Here we have to negate the X- and Y-coordinates because of a bug of the
  // MTC-library.
  returnValue.push_back(-Position[0]);
  returnValue.push_back(-Position[1]);
  returnValue.push_back(Position[2]);

  return returnValue;
}


std::vector<double> mitk::ClaronInterface::GetTipQuaternions(claronToolHandle c)
{
  std::vector<double> returnValue;

  mtHandle t2m = Xform3D_New(); // tooltip to marker xform handle
  mtHandle t2c = Xform3D_New(); // tooltip to camera xform handle
  mtHandle m2c = Xform3D_New(); // marker to camera xform handle

  //Get m2c
  MTC( Marker_Marker2CameraXfGet (c, CurrCamera, m2c, &IdentifyingCamera) );
  //Get t2m
  MTC( Marker_Tooltip2MarkerXfGet (c,  t2m ));
  //Transform both to t2c
  MTC(Xform3D_Concatenate(t2m,m2c,t2c));

  //get the Claron-Quaternion
  double Quarternions[4];
  MTC( Xform3D_RotQuaternionsGet(t2c, Quarternions) );
  mitk::Quaternion claronQuaternion;
  //note: claron quarternion has different order than the mitk quarternion
  claronQuaternion[3] = Quarternions[0];
  claronQuaternion[0] = Quarternions[1];
  claronQuaternion[1] = Quarternions[2];
  claronQuaternion[2] = Quarternions[3];

  // Here we have to make a -90°-turn around the Y-axis because of a bug of the
  // MTC-library.
  mitk::Quaternion minusNinetyDegreeY;
  minusNinetyDegreeY[3] = sqrt(2.0)/2.0;
  minusNinetyDegreeY[0] = 0;
  minusNinetyDegreeY[1] = -1.0/(sqrt(2.0));
  minusNinetyDegreeY[2] = 0;

  //calculate the result...
  mitk::Quaternion erg = (minusNinetyDegreeY*claronQuaternion);

  returnValue.push_back(erg[3]);
  returnValue.push_back(erg[0]);
  returnValue.push_back(erg[1]);
  returnValue.push_back(erg[2]);

  return returnValue;
}

std::vector<double> mitk::ClaronInterface::GetQuaternions(claronToolHandle c)
{
  std::vector<double> returnValue;
  //TODO by Alfred 3.3.2009: I'm afraid we have a bug here. Look at the method
  //                         GetTipQuaternions... the same has to be done here
  //                         to compensate the bug in the MTC-lib.
  double	Quarternions[4];
  MTC( Marker_Marker2CameraXfGet (c, CurrCamera, PoseXf, &IdentifyingCamera) );
  MTC( Xform3D_RotQuaternionsGet(PoseXf, Quarternions) );

  for (int i = 0; i<4; i++) returnValue.push_back(Quarternions[i]);

  return returnValue;
}



const char* mitk::ClaronInterface::GetName(claronToolHandle c)
{
  char MarkerName[MT_MAX_STRING_LENGTH];
  MTC( Marker_NameGet(c, MarkerName, MT_MAX_STRING_LENGTH, 0) );
  std::string returnValue = std::string(MarkerName);
  return MarkerName;
}

bool mitk::ClaronInterface::IsTracking()
{
  return this->isTracking;
}

bool mitk::ClaronInterface::IsMicronTrackerInstalled()
  {
  return true;
  }
