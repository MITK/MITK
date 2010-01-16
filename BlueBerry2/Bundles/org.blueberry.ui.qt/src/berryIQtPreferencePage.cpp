/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryIQtPreferencePage.h"

namespace berry {

void IQtPreferencePage::CreateControl(void* parent)
{
  this->CreateQtControl(static_cast<QWidget*>(parent));
}

void* IQtPreferencePage::GetControl() const
{
  return (void*)this->GetQtControl();
}

}
