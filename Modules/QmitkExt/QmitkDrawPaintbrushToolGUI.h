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

#ifndef QmitkDrawPaintbrushToolGUI_h_Included
#define QmitkDrawPaintbrushToolGUI_h_Included

#include "QmitkPaintbrushToolGUI.h"
#include "QmitkExtExports.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::PaintbrushTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/

class QmitkExt_EXPORT QmitkDrawPaintbrushToolGUI : public QmitkPaintbrushToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkDrawPaintbrushToolGUI, QmitkPaintbrushToolGUI);
    itkNewMacro(QmitkDrawPaintbrushToolGUI);

    virtual ~QmitkDrawPaintbrushToolGUI();

  signals:

  public slots:

  protected slots:

  protected:

    QmitkDrawPaintbrushToolGUI();
};

#endif

