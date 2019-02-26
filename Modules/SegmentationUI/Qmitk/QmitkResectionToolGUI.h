#pragma once

#include <QPushButton>
#include <QLayout>

#include <MitkSegmentationUIExports.h>

#include "mitkResectionTool.h"
#include "QmitkToolGUI.h"

class MITKSEGMENTATIONUI_EXPORT QmitkResectionToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkResectionToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:
  void OnNewToolAssociated(mitk::Tool* tool);

  void OnToolStateChanged(mitk::ResectionTool::ResectionState state);

protected:
  QmitkResectionToolGUI();
  virtual ~QmitkResectionToolGUI();

  mitk::ResectionTool::Pointer m_ResectionTool;

private:
  QPushButton* m_CutInsideButton;
  QPushButton* m_CutOutsideButton;
};
