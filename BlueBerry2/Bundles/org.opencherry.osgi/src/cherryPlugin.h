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

#ifndef CHERRYPLUGIN_H_
#define CHERRYPLUGIN_H_

#include "cherryOSGiDll.h"

#include "osgi/framework/IBundleActivator.h"
#include "osgi/framework/IBundleContext.h"

#include "Poco/Logger.h"
#include "Poco/Path.h"

namespace cherry {

using namespace osgi::framework;


class BundleLoader;

class CHERRY_OSGI Plugin : public IBundleActivator
{
public:
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);

  SmartPointer<osgi::framework::IBundle> GetBundle();
  //Poco::Logger& GetLog();

  bool GetStatePath(Poco::Path& path);

protected:

  friend class BundleLoader;

  //TODO WeakPointer!!!
  SmartPointer<osgi::framework::IBundle> m_Bundle;

};

}

#endif /*CHERRYPLUGIN_H_*/
