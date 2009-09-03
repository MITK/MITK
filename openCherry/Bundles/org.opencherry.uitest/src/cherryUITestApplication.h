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


#ifndef CHERRYUITESTAPPLICATION_H_
#define CHERRYUITESTAPPLICATION_H_

#include <cherryIApplication.h>

namespace cherry {

class UITestApplication : public IApplication
{

public:

  int Start();

  void Stop();

private:

  /*
   * return the application to run, or null if not even the default application
   * is found.
   */
//  IApplication::Pointer GetApplication() throw(CoreException) {
//
//    const IExtension* extension =
//    Platform::GetExtensionPointService()->GetExtension(
//        Platform::PI_RUNTIME,
//        Platform::PT_APPLICATIONS,
//        getApplicationToRun(args));
//  }
};

}

#endif /* CHERRYUITESTAPPLICATION_H_ */
