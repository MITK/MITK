/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkToolGUI_h_Included
#define QmitkToolGUI_h_Included

#include <qwidget.h>
#include "QmitkExtExports.h"

#include "mitkCommon.h"
#include "mitkTool.h"

/**
  \brief Base class for GUIs belonging to mitk::Tool classes.

  \ingroup org_mitk_gui_qt_interactivesegmentation

  Created through ITK object factory. TODO May be changed to a toolkit specific way later?

  Last contributor: $Author$
*/

class QmitkExt_EXPORT QmitkToolGUI : public QWidget, public itk::Object
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkToolGUI, itk::Object);

    void SetTool( mitk::Tool* tool );

    // just make sure ITK won't take care of anything (especially not destruction)
    virtual void Register() const;
    virtual void UnRegister() const;
    virtual void SetReferenceCount(int);

    virtual ~QmitkToolGUI();

  signals:

    void NewToolAssociated( mitk::Tool* );

  public slots:

  protected slots:

  protected:

    mitk::Tool::Pointer m_Tool;
};

#endif

