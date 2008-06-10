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

#include <Poco/ClassLibrary.h>

#include <org.opencherry.osgi/application/cherryIApplication.h>
#include <org.opencherry.ui/src/cherryIEditorPart.h>

#include "src/mitkQtApplication.h"
#include "src/mitkStdMultiWidgetEditor.h"

POCO_BEGIN_NAMED_MANIFEST(cherryIApplication, cherry::IApplication)
  POCO_EXPORT_CLASS(mitkQtApplication)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(cherryIEditorPart, cherry::IEditorPart)
  POCO_EXPORT_CLASS(mitkStdMultiWidgetEditor)
POCO_END_MANIFEST
