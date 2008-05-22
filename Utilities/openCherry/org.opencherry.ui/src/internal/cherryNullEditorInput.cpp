/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryNullEditorInput.h"

namespace cherry
{

NullEditorInput::NullEditorInput()
{
}

NullEditorInput::NullEditorInput(EditorReference::Pointer editorReference)
{
  //poco_assert(editorReference.IsNotNull());
  this->editorReference = editorReference;
}

bool NullEditorInput::Exists()
{
  return false;
}

std::string NullEditorInput::GetName()
{
  if (editorReference.IsNotNull())
    return editorReference->GetName();
  return ""; //$NON-NLS-1$
}

std::string NullEditorInput::GetToolTipText()
{
  if (editorReference.IsNotNull())
    return editorReference->GetTitleToolTip();
  return ""; //$NON-NLS-1$
}

}
