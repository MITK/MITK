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

#include <cherryIApplication.h>

#include "src/QmitkApplication.h"
#include "src/QmitkDefaultPerspective.h"

POCO_BEGIN_NAMED_MANIFEST(cherryIApplication, cherry::IApplication)
  POCO_EXPORT_CLASS(QmitkApplication)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(cherryIPerspectiveFactory, cherry::IPerspectiveFactory)
  POCO_EXPORT_CLASS(QmitkDefaultPerspective)
POCO_END_MANIFEST
