/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <berryLog.h>

#include "berryAbstractSourceProvider.h"
#include "services/berryIServiceLocator.h"

#include <Poco/Exception.h>

namespace berry
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
    BERRY_INFO << "SOURCES" << " >>> " << message;
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

void AbstractSourceProvider::Initialize(IServiceLocator::ConstPointer  /*locator*/)
{
}

}
