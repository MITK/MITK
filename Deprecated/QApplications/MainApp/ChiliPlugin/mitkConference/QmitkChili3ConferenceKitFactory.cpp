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

#include <QmitkChili3ConferenceKitFactory.h>
#include <QmitkChili3ConferenceKit.h>


Chili3ConferenceKitFactory::Chili3ConferenceKitFactory()
{
  mitk::ConferenceKit::SetFactory( this );
}

Chili3ConferenceKitFactory::~Chili3ConferenceKitFactory(){};

mitk::ConferenceKit*
Chili3ConferenceKitFactory::CreateConferenceKit()
{
     return new Chili3Conference;
}

