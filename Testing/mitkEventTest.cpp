/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include <mitkEvent.h>
#include <mitkOpenGLRenderer.h>

#include <fstream>
int mitkEventTest(int /*argc*/, char* /*argv*/[])
{
  mitk::OpenGLRenderer::Pointer renderer = new mitk::OpenGLRenderer; 
  
  //Create Event
  mitk::Event * event = new mitk::Event(renderer, 0, 1, 2, 3);
  
  //check Get...
  std::cout << "check the get methods of the Event";
  if (event->GetSender() != renderer ||
    event->GetType() != 0 ||
    event->GetButton() != 1 ||
    event->GetButtonState() != 2 ||
    event->GetKey() != 3)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
