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

#include "berryIEditorRegistry.h"

namespace berry
{

const int IEditorRegistry::PROP_CONTENTS = 0x01;
const std::string IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID =
    "org.blueberry.ui.systemExternalEditor"; //$NON-NLS-1$
const std::string IEditorRegistry::SYSTEM_INPLACE_EDITOR_ID =
    "org.blueberry.ui.systemInPlaceEditor"; //$NON-NLS-1$

IEditorRegistry::~IEditorRegistry()
{
}

}
