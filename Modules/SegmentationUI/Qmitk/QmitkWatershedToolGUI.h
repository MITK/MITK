/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWatershedToolGUI_h_Included
#define QmitkWatershedToolGUI_h_Included

#include "QmitkAutoMLSegmentationToolGUIBase.h"

#include "ui_QmitkWatershedToolGUIControls.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::WatershedTool.
  \sa mitk::WatershedTool

  This GUI shows two sliders to change the watershed parameters. It executes the watershed algorithm by clicking on the
  button.

*/
class MITKSEGMENTATIONUI_EXPORT QmitkWatershedToolGUI : public QmitkAutoMLSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkWatershedToolGUI, QmitkAutoMLSegmentationToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :

  void OnSettingsAccept();

  void OnLevelChanged(double value);
  void OnThresholdChanged(double value);

protected:
  QmitkWatershedToolGUI();
  ~QmitkWatershedToolGUI() = default;

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool enabled) override;

  //Recommendation from ITK is to have a threshold:level ration around 1:100
  //we set Level a bit higher. This provokes more oversegmentation,
  //but produces less objects in the first run and profits form the fact that
  //decreasing level is quite fast in the filter.
  double m_Level = 0.6;
  double m_Threshold = 0.004;

  Ui_QmitkWatershedToolGUIControls m_Controls;
};

#endif
