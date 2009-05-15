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

#include <mitkProperties.h>

class QWidget;

class QMITKEXT_EXPORT QmitkPropertyViewFactory
{
  public:

    enum ViewTypes { vtDEFAULT = 0  };
    enum EditorTypes { etDEFAULT = 0, etALWAYS_EDIT = 1, etON_DEMAND_EDIT = 2 };

    static QmitkPropertyViewFactory* GetInstance(); // singleton

    /// Views and editors are created via operator new. After creation they belong to the caller of CreateView()/CreateEditor(),
    /// i.e. the caller has to ensure, that the objects are properly deleted!
    QWidget* CreateView  (const mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0);
    QWidget* CreateEditor(mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0);

  protected:

    QmitkPropertyViewFactory();   // hidden, access through GetInstance()
    ~QmitkPropertyViewFactory();

  private:

};

