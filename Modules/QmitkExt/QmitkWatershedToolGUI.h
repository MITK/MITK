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

#ifndef QmitkWatershedToolGUI_h_Included
#define QmitkWatershedToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "QmitkExtExports.h"
#include "mitkWatershedTool.h"

class QSlider;
class QLabel;
class QFrame;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::WatershedTool.
  \sa mitk::WatershedTool

  This GUI shows two sliders to change the watershed parameters.

  Last contributor: $Author$
*/
class QmitkExt_EXPORT QmitkWatershedToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkWatershedToolGUI, QmitkToolGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnSliderValueThresholdChanged(int value);
    void OnSliderValueLevelChanged(int value);
    void OnCreateSegmentation();

  protected:

    QmitkWatershedToolGUI();
    virtual ~QmitkWatershedToolGUI();

    QSlider* m_SliderThreshold;
    QSlider* m_SliderLevel;

    QLabel* m_ThresholdLabel;
    QLabel* m_LevelLabel;
    QFrame* m_Frame;

    mitk::WatershedTool::Pointer m_WatershedTool;
};

#endif
