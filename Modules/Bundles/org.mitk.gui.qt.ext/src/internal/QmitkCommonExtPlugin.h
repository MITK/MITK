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


#ifndef QMITKCOMMONEXTPLUGIN_H_
#define QMITKCOMMONEXTPLUGIN_H_

#include <berryPlugin.h>

class QmitkCommonExtPlugin : public berry::Plugin
{
public:

  void Start(berry::IBundleContext::Pointer context);

};

#endif /* QMITKCOMMONEXTPLUGIN_H_ */
