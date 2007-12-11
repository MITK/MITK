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

#include <mitkChiliConferenceKitFactory.h>
#include <QmitkChili3ConferenceKit.h>

namespace mitk{
//class ConferenceKit;

//ConferenceKit::Pointer ChiliConferenceKitFactory::m_QCInstance;

ChiliConferenceKitFactory::ChiliConferenceKitFactory()
{
  ConferenceKit::SetFactory( this );
};

ChiliConferenceKitFactory::~ChiliConferenceKitFactory(){};

ConferenceKit*
ChiliConferenceKitFactory::CreateConferenceKit()
{
//   QChili3Conference::Pointer qcc = QChili3Conference::New();
//   return qcc->GetPointer();
     return new QChili3Conference;
};

}//namespace
