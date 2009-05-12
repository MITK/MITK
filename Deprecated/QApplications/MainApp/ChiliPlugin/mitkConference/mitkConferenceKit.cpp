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

#include "mitkConferenceKit.h"

namespace mitk{

  ConferenceKit* ConferenceKit::m_Instance = NULL;

  ConferenceKitFactory* ConferenceKit::m_ConferenceKitFactory = NULL;

  /* Set the factory instance for further use.
   */
  void ConferenceKit::SetFactory( ConferenceKitFactory* factory )
  {
    m_ConferenceKitFactory = factory;
  };

  /* Returns the ConferenceKit instance to create the specific products.
   */
  ConferenceKit*
  ConferenceKit::GetInstance()
  {
    if ( ! m_Instance )
    {
      if (m_ConferenceKitFactory)
      {
          m_Instance = m_ConferenceKitFactory->CreateConferenceKit();
      }
    }
    return m_Instance;
  }

}//namespace
