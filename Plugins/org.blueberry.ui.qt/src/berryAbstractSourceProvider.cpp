/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryLog.h>

#include "berryAbstractSourceProvider.h"
#include "services/berryIServiceLocator.h"

#include <Poco/Exception.h>

namespace berry
{

bool AbstractSourceProvider::DEBUG = false; //Policy.DEBUG_SOURCES;

void AbstractSourceProvider::FireSourceChanged(int sourcePriority,
    const QString &sourceName, Object::ConstPointer sourceValue)
{
  sourceEvents.singleSourceChanged(sourcePriority, sourceName, sourceValue);
}

void AbstractSourceProvider::FireSourceChanged(int sourcePriority,
    const QHash<QString, Object::ConstPointer> &sourceValuesByName)
{
  sourceEvents.multipleSourcesChanged(sourcePriority, sourceValuesByName);
}

void AbstractSourceProvider::LogDebuggingInfo(const QString& message)
{
  if (DEBUG && (message != ""))
  {
    BERRY_INFO << "SOURCES >>> " << message;
  }
}

void AbstractSourceProvider::AddSourceProviderListener(
    ISourceProviderListener* listener)
{
  if (listener == nullptr)
  {
    throw Poco::NullPointerException("The listener cannot be null");
  }

  sourceEvents.AddListener(listener);
}

void AbstractSourceProvider::RemoveSourceProviderListener(
    ISourceProviderListener* listener)
{
  if (listener == nullptr)
  {
    throw Poco::NullPointerException("The listener cannot be null");
  }

  sourceEvents.RemoveListener(listener);
}

void AbstractSourceProvider::Initialize(IServiceLocator* /*locator*/)
{
}

}
