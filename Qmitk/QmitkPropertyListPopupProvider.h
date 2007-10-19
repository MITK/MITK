/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkPropertyListPopupProvider_h_Included
#define QmitkPropertyListPopupProvider_h_Included

#include "mitkCommon.h"
#include <itkObjectFactory.h>

namespace mitk
{
  class PropertyList;
}

class QObject;
class QmitkPropertyListPopup;

/**
  \brief Factory method to create PropertyListPopups

  Provides one method CreatePopup(...) to create a popup menu from a PropertyList. This class is an itk::Object 
  and can be exchanged with a different implementation by means of the ITK factory. This is useful to create
  different kinds of popups (e.g. some popup that depends on closed source code).

  $Author$
*/
class QmitkPropertyListPopupProvider : public itk::Object
{
  public:
    typedef QmitkPropertyListPopupProvider Self;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(QmitkPropertyListPopupProvider);

    QmitkPropertyListPopup* CreatePopup(mitk::PropertyList*, QObject* parent = 0, bool disableBoolProperties = false, const char* name = 0);

  protected:

    QmitkPropertyListPopupProvider(); // purposely hidden
    virtual ~QmitkPropertyListPopupProvider();
};

#endif

