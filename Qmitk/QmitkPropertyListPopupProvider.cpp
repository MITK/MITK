/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkPropertyListPopupProvider.h"

#include "QmitkPropertyListPopup.h"

QmitkPropertyListPopupProvider::QmitkPropertyListPopupProvider()
{
}

QmitkPropertyListPopupProvider::~QmitkPropertyListPopupProvider()
{
}

QmitkPropertyListPopup* QmitkPropertyListPopupProvider::CreatePopup(mitk::PropertyList* list, QObject* parent, bool disableBoolProperties, const char* name)
{
  return new QmitkPropertyListPopup(list, parent, disableBoolProperties, name);
}
