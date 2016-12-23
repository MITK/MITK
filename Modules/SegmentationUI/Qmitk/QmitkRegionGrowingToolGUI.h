#pragma once

#include <mitkRegionGrowingTool.h>

#include "QmitkToolGUI.h"
#include "ui_QmitkRegionGrowingToolGUIControls.h"

class RegionGrowingTool;

class MITKSEGMENTATIONUI_EXPORT QmitkRegionGrowingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkRegionGrowingToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

protected slots:
  void OnNewToolAssociated(mitk::Tool* tool);

protected:

  QmitkRegionGrowingToolGUI();
  virtual ~QmitkRegionGrowingToolGUI();

  void onThresholdsChanged(double lowThreshold, double highThreshold);

  mitk::RegionGrowingTool::Pointer m_RegionGrowingTool;
  Ui::QmitkRegionGrowingToolGUIControls m_Controls;
};
