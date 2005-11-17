/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

class QmitkPropertyViewFactory
{
  public:

    typedef enum ViewTypes { DEFAULT = 0  };
    typedef enum EditorTypes { DEFAULT = 0  };

    static QmitkPropertyViewFactory* GetInstance(); // singleton

    // Views and editors are created via operator new. After creation they belong to the caller of CreateView()/CreateEditor(),
    // i.e. the caller has to ensure, that the objects are properly deleted!
    QWidget* CreateView  (mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0, const char* name = 0);
    QWidget* CreateEditor(mitk::BaseProperty* property, unsigned int type = 0, QWidget* parent = 0, const char* name = 0);

  protected:

    QmitkPropertyViewFactory();   // hidden, access through GetInstance()
    ~QmitkPropertyViewFactory();

  private:

};

