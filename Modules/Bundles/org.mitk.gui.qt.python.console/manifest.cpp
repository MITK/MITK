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

#include <Poco/ClassLibrary.h>


#include "src/internal/QmitkPythonEditor.h"
#include "src/internal/QmitkPythonConsoleView.h"
#include "src/internal/QmitkPythonVariableStack.h"
#include "src/internal/QmitkPythonCommandHistory.h"
#include "src/internal/QmitkPythonPerspective.h"

POCO_BEGIN_NAMED_MANIFEST(berryIEditorPart, berry::IEditorPart)
    POCO_EXPORT_CLASS(::QmitkPythonEditor)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(::QmitkPythonConsoleView)
  POCO_EXPORT_CLASS(::QmitkPythonVariableStack)
  POCO_EXPORT_CLASS(::QmitkPythonCommandHistory)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIPerspectiveFactory, berry::IPerspectiveFactory)
   POCO_EXPORT_CLASS(QmitkPythonPerspective)
POCO_END_MANIFEST
