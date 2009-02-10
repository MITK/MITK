// TrackingExample - Shows how to construct a tracking system

#include <Framework/RS232/RS232int.h>

#include "mitkTrackingTypes.h"
#include "mitkNDITrackingDevice.h"
#include "mitkDaVinciTrackingDevice.h"
#include "mitkNDIPassiveTool.h"
#include "mitkVector.h"
#include <itksys/SystemTools.hxx>

/*Begining of defining important variables:*/

/*
SromFiles: "\" has to be terminated with an other "\"
usual files are:
"C:\\ndigital\\Passive 4 Marker Planar Rigid Body Kit\\8700302.rom"
"C:\\ndigital\\daVinci-OP Markerconfig\\daVinci.rom"
"C:\\ndigital\\daVinci-OP Markerconfig\\patient.rom"
"C:\\ndigital\\daVinci-OP Markerconfig\\liege.rom"
*/
const char* SROMFILE1 = "C:\\NDIgitalRomFiles\\polaris\\stretcher.rom"; 
const char* SROMFILE2 = "C:\\NDIgitalRomFiles\\polaris\\daVinci.rom";
const char* SROMFILE5D = "C:\\ndigital\\Passive 4 Marker Planar Rigid Body Kit\\8700302.rom";
const char* SROMFILEAURORA = "C:\\NDIgitalRomFiles\\aurora\\6DDUALBRONCHOSENSOR.ROM";

//switch to load srom-files 
bool LOADFILE = true;
//which port to load file to
unsigned int PORTNUMBERTOLOAD = 1;

//how many times shall the tracking be done?
unsigned int NUMBEROFTRACKINGSTEPS = 120;

//TrackingDeviceType
mitk::NDITrackingDevice::NDITrackingDeviceType trackingDevice = 
        mitk::NDIAurora;

//ComPortNumber:
mitk::PortNumber portNumber = mitk::COM1;
//also used: mitk::COM4

//test tracking of polaris or aurora in usual mode
void TestTracking();

//test polaris with reimplemented 5D tracking
void TestMarkerTracking();

// test daVinci tracking device
void TestDaVinciTracking();

int main()
{
  TestDaVinciTracking();  // <-- daVinci
  //TestTracking();
  return 0;
}



void TestTracking()
{
  // instantiate tracker
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  //use comport 1 as default
  tracker->SetPortNumber(portNumber);
  //set highest baudrate and databits
  tracker->SetBaudRate(mitk::BaudRate115200);
  tracker->SetDataBits(mitk::DataBits8);

  //if Polaris, add passive tools
  switch (trackingDevice)
  {
  case mitk::NDIPolaris:
    {
      tracker->SetType(mitk::NDIPolaris);

      /* Create tools with different srom files */
      mitk::NDIPassiveTool::Pointer tool1 = mitk::NDIPassiveTool::New();
      tool1->SetToolName("stretcher");
      tool1->LoadSROMFile(SROMFILE1);
      tool1->SetTrackingPriority(mitk::Dynamic);

      mitk::NDIPassiveTool::Pointer tool2 = mitk::NDIPassiveTool::New();
      tool2->SetToolName("daVinci Tool");
      tool1->LoadSROMFile(SROMFILE2);
      tool2->SetTrackingPriority(mitk::Dynamic);

      /* add tools to the tracking system */
      tracker->Add6DTool(tool1);
      tracker->Add6DTool(tool2);

    }
  
  //if Aurora, connected tools will be set up and srom-files can be loaded afterwards
  case mitk::NDIAurora:
    {
      tracker->SetType(mitk::NDIAurora);
    }
  }

  //open the connection, load tools that are connected (aktive Tools) and initialize them.
  if (tracker->OpenConnection() == false)
  {
    std::cout << tracker->GetErrorMessage();
    return;
  }
  else
    std::cout << "\n \n System initialisiert!";


  std::cout<<"Anzahl Tools: "<<tracker->GetToolCount()<<std::endl;


  //after initialization and enabling tools, load an srom-file manually
  if (LOADFILE)
  {
    unsigned int size = tracker->GetToolCount();
    if (size > 0 && PORTNUMBERTOLOAD < size)
    {
      mitk::TrackingTool* t = tracker->GetTool(PORTNUMBERTOLOAD);
      mitk::NDIPassiveTool* passiveTool = dynamic_cast<mitk::NDIPassiveTool*>(t);
      if (passiveTool != NULL)
      {
        passiveTool->LoadSROMFile(SROMFILEAURORA);
        bool ok = tracker->UpdateTool(passiveTool);
      }
    }
  }  
    
  // start tracking 
  tracker->SetDataTransferMode(mitk::TX);
  
  //signalize tracking
  tracker->Beep(1);
  
  std::cout << "Starting Tracking" << std::endl;
  tracker->StartTracking();

  // read a few times the transformation data of all tools and display them on the console 
  mitk::TrackingTool* t = NULL;
  mitk::Point3D pos;
  mitk::Quaternion quat;
  
  //do the tracking the given times
  for(int i=0; i<NUMBEROFTRACKINGSTEPS; i++)
  {
    for (unsigned int i = 0; i < tracker->GetToolCount(); i++)
    {
      t = tracker->GetTool(i);
      if (t == NULL)
        continue;
      std::cout << t->GetToolName() << ":" << std::endl;
      if (t->IsDataValid() == true)
      {
        t->GetPosition(pos);
        std::cout << "  Position      = <" << pos[0] << ", " << pos[1] << ", " << pos[2] << ">" << std::endl; 
        t->GetQuaternion(quat);
        std::cout << "  Orientation   = <" << quat[0] << ", " << quat[1] << ", " << quat[2] << ", " << quat[3] << ">" << std::endl;
        std::cout << "  TrackingError = " << t->GetTrackingError() << std::endl;
      }
      else
        std::cout << "  Data is invalid. Errormessage: " << t->GetErrorMessage() << std::endl << std::endl << std::endl;

      std::cout << "--------------------------------------------" << std::endl;
    }
    
    //wait a little to get the next coordinate
    itksys::SystemTools::Delay(100);
  }
  

  // stop tracking 
  tracker->StopTracking();
  
  // beep before closing the connection 
  tracker->Beep(2);

  // close the connection to the tracking system device 
  tracker->CloseConnection();

  return;
}


void TestMarkerTracking()
{
  /* test 3D marker tracking */

 /* Create and initialize tracking system */
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  tracker->SetType(mitk::NDIPolaris);
  tracker->SetPortNumber(portNumber);
  tracker->SetBaudRate(mitk::BaudRate115200);
  tracker->SetDataBits(mitk::DataBits8);
  /* connect the tracking system object to the tracking system device */
  if (tracker->OpenConnection() == false)
  {
    std::cout << tracker->GetErrorMessage();
    return;
  }

  mitk::NDIPassiveTool::Pointer tool1 = mitk::NDIPassiveTool::New();
  tool1->SetToolName("Intuitives Black Board");
  tool1->LoadSROMFile(SROMFILE5D);
  tool1->SetTrackingPriority(mitk::Dynamic);
  tracker->Add6DTool(tool1);

  /* Change to 3D marker tracking */
  tracker->SetOperationMode(mitk::MarkerTracking3D);
  mitk::MarkerPointContainerType markerPoints;

  /* start tracking */
   std::cout << "Starting Tracking" << std::endl;
  tracker->StartTracking();
  while(true)
  {
    itksys::SystemTools::Delay(1000);
    tracker->GetMarkerPositions(&markerPoints);
    std::cout << "Found " << markerPoints.size() << " markers:" << std::endl;
    for (unsigned int i = 0; i < markerPoints.size(); i++)
      std::cout << "Marker " << i << ": " << markerPoints.at(i) << std::endl;
  }
}

TestDaVinciTracking()
{
  /* initialize daVinci tracking system */
  mitk::DaVinciTrackingDevice::Pointer tracker = mitk::DaVinciTrackingDevice::New();
  tracker->SetHostName("localhost");
  tracker->SetPortNumber(5002);
  tracker->SetPassword("XXX");  // Intuitive API Server accepts all passwords, daVinci needs real API password
  /* connect to daVinci and start tracking */
  if (tracker->OpenConnection() == false)
    return;
  tracker->StartTracking();
  /* output position data for two tools to console */
  while (true)
  {
    mitk::Point3D p;
    mitk::DaVinciTool::Pointer t = m_TrackingDevice->GetDaVinciTool(mitk::PSM1);  // get patient side manipulator 1 tool
    t->GetPosition(p);
    std::cout << t->GetToolName() << ": " << p << "." << std::endl;
    t = m_TrackingDevice->GetDaVinciTool(mitk::MTML);   // get left master target manipulator
    t->GetPosition(p);
    std::cout << t->GetToolName() << ": " << p << "." << std::endl;

    std::cout << "--------------------------------------------" << std::endl;    
    //wait a little to get the next coordinate
    itksys::SystemTools::Delay(100);
  }
}
