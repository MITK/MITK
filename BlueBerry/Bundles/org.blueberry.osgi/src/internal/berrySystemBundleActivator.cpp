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

#include "berryLog.h"

#include "berrySystemBundleActivator.h"

#include "berryIBundleContext.h"
#include "berryPlatform.h"
#include "service/berryServiceRegistry.h"

#include "berrySystemBundle.h"
#include "berryExtensionPointService.h"

namespace berry {

void SystemBundleActivator::Start(IBundleContext::Pointer context)
{

}

void SystemBundleActivator::Stop(IBundleContext::Pointer /*context*/)
{

}

}
