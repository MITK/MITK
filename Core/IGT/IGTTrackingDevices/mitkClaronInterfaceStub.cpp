#include <mitkClaronInterfaceStub.h>
#include <string>

mitk::ClaronInterface::ClaronInterface(std::string calibrationDir, std::string toolFilesDir)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  }

bool mitk::ClaronInterface::StartTracking()
  {
  printf("Error: MicronTracker is not included in this project. \n");
  return false;
  }

bool mitk::ClaronInterface::StopTracking()
  {
  printf("Error: MicronTracker is not included in this project. \n");
  return false;
  }

std::vector<mitk::claronToolHandle> mitk::ClaronInterface::GetAllActiveTools()
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<mitk::claronToolHandle> returnValue;
  return returnValue;
  }

void mitk::ClaronInterface::GrabFrame()
  {
  printf("Error: MicronTracker is not included in this project. \n");
  }

std::vector<double> mitk::ClaronInterface::GetTipPosition(mitk::claronToolHandle c)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

std::vector<double> mitk::ClaronInterface::GetPosition(claronToolHandle c)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }


std::vector<double> mitk::ClaronInterface::GetTipQuaternions(claronToolHandle c)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

std::vector<double> mitk::ClaronInterface::GetQuaternions(claronToolHandle c)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

std::string mitk::ClaronInterface::GetName(claronToolHandle c)
  {
  printf("Error: MicronTracker is not included in this project. \n");
  return NULL;  
  }

bool mitk::ClaronInterface::IsMicronTrackerInstalled()
  {
  return false;
  }
