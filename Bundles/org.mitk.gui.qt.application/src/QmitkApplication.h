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

#ifndef QMITKAPPLICATION_H_
#define QMITKAPPLICATION_H_

#include <application/cherryIApplication.h>

#include "mitkQtAppDll.h"

class MITK_QT_APP QmitkApplication : public cherry::IApplication
{
public:
  
  int Start();
  void Stop();
};

#endif /*QMITKAPPLICATION_H_*/
