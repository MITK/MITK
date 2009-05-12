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

#include "cherryIApplication.h"

namespace cherry
{

const int IApplication::EXIT_OK = 0;
const int IApplication::EXIT_RESTART = 23;
const int IApplication::EXIT_RELAUNCH = 24;

IApplication::~IApplication()
{
}

}
