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

#include "berryQtDnDTweaklet.h"

#include "berryQtTracker.h"

namespace berry {

QtDnDTweaklet::QtDnDTweaklet()
{

}

QtDnDTweaklet::QtDnDTweaklet(const QtDnDTweaklet& other)
{
  Q_UNUSED(other)
}

ITracker* QtDnDTweaklet::CreateTracker()
{
  return new QtTracker();
}

}
