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

mitk::ClaronInterface::ClaronInterface(std::string itkNotUsed(calibrationDir), std::string itkNotUsed(toolFilesDir))
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

std::vector<double> mitk::ClaronInterface::GetTipPosition(mitk::claronToolHandle itkNotUsed(c))
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

std::vector<double> mitk::ClaronInterface::GetPosition(claronToolHandle itkNotUsed(c))
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }


std::vector<double> mitk::ClaronInterface::GetTipQuaternions(claronToolHandle itkNotUsed(c))
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

std::vector<double> mitk::ClaronInterface::GetQuaternions(claronToolHandle itkNotUsed(c))
  {
  printf("Error: MicronTracker is not included in this project. \n");
  std::vector<double> returnValue;
  return returnValue;
  }

const char* mitk::ClaronInterface::GetName(claronToolHandle itkNotUsed(c))
  {
  printf("Error: MicronTracker is not included in this project. \n");
  return NULL;  
  }

bool mitk::ClaronInterface::IsMicronTrackerInstalled()
  {
  return false;
  }

