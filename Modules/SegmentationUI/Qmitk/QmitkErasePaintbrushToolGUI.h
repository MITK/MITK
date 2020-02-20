/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkErasePaintbrushToolGUI_h_Included
#define QmitkErasePaintbrushToolGUI_h_Included

#include "QmitkPaintbrushToolGUI.h"
#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::PaintbrushTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/

class MITKSEGMENTATIONUI_EXPORT QmitkErasePaintbrushToolGUI : public QmitkPaintbrushToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkErasePaintbrushToolGUI, QmitkPaintbrushToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    signals :

    public slots :

    protected slots :

    protected :

    QmitkErasePaintbrushToolGUI();
  ~QmitkErasePaintbrushToolGUI() override;
};

#endif
