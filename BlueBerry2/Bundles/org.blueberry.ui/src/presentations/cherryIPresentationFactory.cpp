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

#include "cherryIPresentationFactory.h"

#include "../cherryConstants.h"

namespace cherry
{

int IPresentationFactory::SASHTYPE_NORMAL = 0;
int IPresentationFactory::SASHTYPE_FLOATING = 1 << 1;
int IPresentationFactory::SASHORIENTATION_HORIZONTAL = Constants::HORIZONTAL; // 1<<8
int IPresentationFactory::SASHORIENTATION_VERTICAL = Constants::VERTICAL; // 1<<9

}
