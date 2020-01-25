/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWatershedToolGUI_h_Included
#define QmitkWatershedToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkWatershedTool.h"
#include <MitkSegmentationUIExports.h>

class QSlider;
class QLabel;
class QFrame;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::WatershedTool.
  \sa mitk::WatershedTool

  This GUI shows two sliders to change the watershed parameters. It executes the watershed algorithm by clicking on the
  button.

*/
class MITKSEGMENTATIONUI_EXPORT QmitkWatershedToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkWatershedToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    protected slots :

    void OnNewToolAssociated(mitk::Tool *);

  /** \brief Passes the chosen threshold value directly to the watershed tool */
  void OnSliderValueThresholdChanged(int value);
  /** \brief Passes the chosen level value directly to the watershed tool */
  void OnSliderValueLevelChanged(int value);
  /** \brief Starts segmentation algorithm in the watershed tool */
  void OnCreateSegmentation();

protected:
  QmitkWatershedToolGUI();
  ~QmitkWatershedToolGUI() override;

  QSlider *m_SliderThreshold;
  QSlider *m_SliderLevel;

  /** \brief Label showing the current threshold value. */
  QLabel *m_ThresholdLabel;
  /** \brief Label showing the current level value. */
  QLabel *m_LevelLabel;
  /** \brief Label showing additional informations. */
  QLabel *m_InformationLabel;

  QFrame *m_Frame;

  mitk::WatershedTool::Pointer m_WatershedTool;
};

#endif
