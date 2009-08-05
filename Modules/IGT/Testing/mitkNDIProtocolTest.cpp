/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNDIProtocol.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**Documentation
* NDIProtocol has a protected constructor and a protected itkNewMacro
* so that only it's friend class NDITrackingDevice is able to instantiate
* tool objects. Therefore, we derive from NDIPassiveTool and add a 
* public itkNewMacro, so that we can instantiate and test the class
*/
class NDIProtocolTestClass : public mitk::NDIProtocol
{
public:
  mitkClassMacro(NDIProtocolTestClass, NDIProtocol);
  /** make a public constructor, so that the test is able
  *   to instantiate NDIPassiveTool
  */
  itkNewMacro(Self);
protected:
  NDIProtocolTestClass() : mitk::NDIProtocol()  
  {
  }
};

/**Documentation
 *  Test for mitk::NDIProtocol
 */
int mitkNDIProtocolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NDIProtocolTest");

  // let's create an object of our class  
  mitk::NDIProtocol::Pointer myNDIProtocol = NDIProtocolTestClass::New().GetPointer();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myNDIProtocol.IsNotNull(),"Testing instantiation");


  //COMM(mitk::SerialCommunication::BaudRate baudRate , mitk::SerialCommunication::DataBits dataBits, mitk::SerialCommunication::Parity parity, mitk::SerialCommunication::StopBits stopBits, mitk::SerialCommunication::HardwareHandshake hardwareHandshake)
  //No testing possible, hardware required



  //All other methods
  //No testing possible, hardware required

  
  
  // always end with this!
  MITK_TEST_END();
}
