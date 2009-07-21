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

#include <mitkClaronInterfaceStub.h>
#include <itkMacro.h>
#include <string>

mitk::ClaronInterface::ClaronInterface()
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
}

mitk::ClaronInterface::~ClaronInterface()
{

}

void mitk::ClaronInterface::Initialize(std::string itkNotUsed(calibrationDir), std::string itkNotUsed(toolFilesDir))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
}

bool mitk::ClaronInterface::StartTracking()
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  return false;
}

bool mitk::ClaronInterface::StopTracking()
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  return false;
}

std::vector<mitk::claronToolHandle> mitk::ClaronInterface::GetAllActiveTools()
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  std::vector<mitk::claronToolHandle> returnValue;
  return returnValue;
}

void mitk::ClaronInterface::GrabFrame()
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
}

std::vector<double> mitk::ClaronInterface::GetTipPosition(mitk::claronToolHandle itkNotUsed(c))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  std::vector<double> returnValue;
  return returnValue;
}

std::vector<double> mitk::ClaronInterface::GetPosition(claronToolHandle itkNotUsed(c))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  std::vector<double> returnValue;
  return returnValue;
}


std::vector<double> mitk::ClaronInterface::GetTipQuaternions(claronToolHandle itkNotUsed(c))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  std::vector<double> returnValue;
  return returnValue;
}

std::vector<double> mitk::ClaronInterface::GetQuaternions(claronToolHandle itkNotUsed(c))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  std::vector<double> returnValue;
  return returnValue;
}

const char* mitk::ClaronInterface::GetName(claronToolHandle itkNotUsed(c))
{
  LOG_WARN("IGT") << "Error: MicronTracker is not included in this project";
  return NULL;  
}

bool mitk::ClaronInterface::IsMicronTrackerInstalled()
{
  return false;
}
