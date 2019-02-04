#pragma once

#include <MitkSegmentationUIExports.h>

#include "mitkSmartBrushTool.h"

#include "QmitkToolGUI.h"

class QSlider;
class QLabel;
class QFrame;

class MITKSEGMENTATIONUI_EXPORT QmitkSmartBrushToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkSmartBrushToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  void OnRadiusChanged(int current);
  void OnSensitivityChanged(int current);

protected slots:
  void OnNewToolAssociated(mitk::Tool* tool);
  void OnRadiusSliderChanged(int value);
  void OnSensitivitySliderChanged(int value);
  void VisualizeRadius(int size);
  void VisualizeSensitivity(int sens);

protected:
  QmitkSmartBrushToolGUI();
  virtual ~QmitkSmartBrushToolGUI();

  QSlider* m_RadiusSlider;
  QLabel* m_RadiusLabel;
  QSlider* m_SensitivitySlider;
  QLabel* m_SensitivityLabel;

  mitk::SmartBrushTool::Pointer m_SmartBrushTool;
};
