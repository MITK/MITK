/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDrawPaintbrushToolGUI_h
#define QmitkDrawPaintbrushToolGUI_h

#include "QmitkPaintbrushToolGUI.h"
#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::PaintbrushTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/

class MITKSEGMENTATIONUI_EXPORT QmitkDrawPaintbrushToolGUI : public QmitkPaintbrushToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkDrawPaintbrushToolGUI, QmitkPaintbrushToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    ~QmitkDrawPaintbrushToolGUI() override;

signals:

public slots:

protected slots:

protected:
  QmitkDrawPaintbrushToolGUI();
};

#endif
