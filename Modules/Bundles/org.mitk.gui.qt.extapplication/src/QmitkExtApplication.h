/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QMITKEXTAPPLICATION_H_
#define QMITKEXTAPPLICATION_H_

#include <berryIApplication.h>

#include "mitkQtExtAppDll.h"

class MITK_QT_EXTAPP QmitkExtApplication : public berry::IApplication
{
public:
  
  int Start();
  void Stop();
};

#endif /*QMITKEXTAPPLICATION_H_*/
