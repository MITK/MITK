/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkBaseController.h"
#include "mitkBaseRenderer.h"

mitk::BaseController::BaseController(const char * type) : StateMachine(type), m_LastUpdateTime(0)
{
  m_Slice = Stepper::New();
  m_Time  = Stepper::New();
}

//##ModelId=3E3AE32B0070
mitk::BaseController::~BaseController()
{
}

//##ModelId=3DF8F5CA03D8
mitk::Stepper* mitk::BaseController::GetSlice()
{
  return m_Slice.GetPointer();
}

//##ModelId=3DF8F61101E3
mitk::Stepper* mitk::BaseController::GetTime()
{
  return m_Time.GetPointer();
}
