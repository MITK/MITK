/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision: 14645 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <cherryIViewPart.h>
#include "src/QmitkImageCropper.h"

POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(QmitkImageCropper)
POCO_END_MANIFEST
