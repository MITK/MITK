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

#include "cherryIBundleActivator.h"
#include "cherryIBundleContext.h"

#include "Poco/Logger.h"
#include "Poco/Path.h"

struct IBundle;
class BundleLoader;

namespace cherry {

class CHERRY_OSGI Plugin : public IBundleActivator
{
public:
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);

  SmartPointer<IBundle> GetBundle();
  //Poco::Logger& GetLog();

  Poco::Path* GetStatePath();

protected:

  friend class BundleLoader;

  //TODO WeakPointer!!!
  SmartPointer<IBundle> m_Bundle;

};

}

#endif /*CHERRYPLUGIN_H_*/
