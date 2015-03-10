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

#ifndef QmitkPaintbrushToolGUI_h_Included
#define QmitkPaintbrushToolGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "mitkPaintbrushTool.h"

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

    void OnNewToolAssociated(mitk::Tool*);

    void OnSliderValueChanged(int value);

    void VisualizePaintbrushSize(int size);

  protected:

    QmitkPaintbrushToolGUI();
    virtual ~QmitkPaintbrushToolGUI();

    QSlider* m_Slider;
    QLabel* m_SizeLabel;
    QFrame* m_Frame;

    mitk::PaintbrushTool::Pointer m_PaintbrushTool;
};

#endif

