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

#include "cherryLog.h"

#include "cherryAbstractSourceProvider.h"
#include "services/cherryIServiceLocator.h"

#include <Poco/Exception.h>

namespace cherry
{

bool AbstractSourceProvider::DEBUG = true; //Policy.DEBUG_SOURCES;

void AbstractSourceProvider::FireSourceChanged(int sourcePriority,
    const std::string& sourceName, Object::Pointer sourceValue)
{
  sourceEvents.singleSourceChanged(sourcePriority, sourceName, sourceValue);
}

void AbstractSourceProvider::FireSourceChanged(int sourcePriority,
    const std::map<std::string, Object::Pointer>& sourceValuesByName)
{
  sourceEvents.multipleSourcesChanged(sourcePriority, sourceValuesByName);
}

void AbstractSourceProvider::LogDebuggingInfo(const std::string& message)
{
  if (DEBUG && (message != ""))
  {
    CHERRY_INFO << "SOURCES" << " >>> " << message;
  }
}

void AbstractSourceProvider::AddSourceProviderListener(
    ISourceProviderListener::Pointer listener)
{
  if (listener == 0)
  {
    throw Poco::NullPointerException("The listener cannot be null"); //$NON-NLS-1$
  }

  sourceEvents.AddListener(listener);
}

void AbstractSourceProvider::RemoveSourceProviderListener(
    ISourceProviderListener::Pointer listener)
{
  if (listener == 0)
  {
    throw Poco::NullPointerException("The listener cannot be null"); //$NON-NLS-1$
  }

  sourceEvents.RemoveListener(listener);
}

void AbstractSourceProvider::Initialize(IServiceLocator::ConstPointer locator)
{
}

}
