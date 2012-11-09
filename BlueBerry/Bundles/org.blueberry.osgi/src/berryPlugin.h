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

#ifndef BERRYPLUGIN_H_
#define BERRYPLUGIN_H_

#include <org_blueberry_osgi_Export.h>

#include "berryIBundleActivator.h"
#include "berryIBundleContext.h"

#include "Poco/Logger.h"
#include "Poco/Path.h"

struct IBundle;
class BundleLoader;

namespace berry {

class BERRY_OSGI Plugin : public IBundleActivator
{
public:
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);

  SmartPointer<IBundle> GetBundle();
  //Poco::Logger& GetLog();

  bool GetStatePath(Poco::Path& path);

protected:

  friend class BundleLoader;

  //TODO WeakPointer!!!
  SmartPointer<IBundle> m_Bundle;

};

}

#endif /*BERRYPLUGIN_H_*/
