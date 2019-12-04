/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPaintbrushToolGUI_h_Included
#define QmitkPaintbrushToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkPaintbrushTool.h"
#include <MitkSegmentationUIExports.h>

class QSlider;
class QLabel;
class QFrame;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::PaintbrushTool.
  \sa mitk::PaintbrushTool

  This GUI shows a slider to change the pen's size.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkPaintbrushToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkPaintbrushToolGUI, QmitkToolGUI);

  void OnSizeChanged(int current);

signals:

public slots:

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnSliderValueChanged(int value);

  void VisualizePaintbrushSize(int size);

protected:
  QmitkPaintbrushToolGUI();
  ~QmitkPaintbrushToolGUI() override;

  QSlider *m_Slider;
  QLabel *m_SizeLabel;
  QFrame *m_Frame;

  mitk::PaintbrushTool::Pointer m_PaintbrushTool;
};

#endif
