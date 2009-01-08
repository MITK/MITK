/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2008-04-24 18:31:42 +0200 (Do, 24 Apr 2008) $
Version:   $Revision: 1.0 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkToolGUI_h_Included
#define QmitkToolGUI_h_Included

#include <qwidget.h>
//!mm
//#include <itkObject.h>
#include <cherryMacros.h>
#include <cherryObject.h>
//!

#include "mitkCommon.h"
#include "mitkTool.h"
#include "mitkQtInteractiveSegmentationDll.h"

/**
  \brief Base class for GUIs belonging to mitk::Tool classes.

  Created through ITK object factory. TODO May be changed to a toolkit specific way later?

  Last contributor: $Author$
*/
//!mm
//class MITK_QT_INTERACTIVESEGMENTATION QmitkToolGUI : public QWidget, public itk::Object
class MITK_QT_INTERACTIVESEGMENTATION QmitkToolGUI : public QWidget, public virtual cherry::Object
//!
{
  Q_OBJECT

  public:
    //!mm
    // class has now a cherryInterfaceMacro
    //mitkClassMacro(QmitkToolGUI, itk::Object);
    cherryInterfaceMacro(QmitkToolGUI, )
    //!

    void SetTool( mitk::Tool* tool );

    //!mm
    // commented out
    /// just make sure ITK won't take care of anything (especially not destruction)
    //virtual void Register() const;
    //virtual void UnRegister() const;
    //virtual void SetReferenceCount(int);
    //!

    virtual ~QmitkToolGUI();

  signals:

    void NewToolAssociated( mitk::Tool* );

  public slots:

  protected slots:

  protected:

    QmitkToolGUI();

    mitk::Tool::Pointer m_Tool;
};

#endif

