/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLabelSelectionToolGUI_h
#define QmitkLabelSelectionToolGUI_h

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>

namespace mitk
{
  class LabelSelectionTool;
}

namespace Ui
{
  class QmitkLabelSelectionToolGUIControls;
}

class QmitkFloatingTextPopup;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::LabelSelectionTool.
  \sa mitk::LabelSelectionTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLabelSelectionToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkLabelSelectionToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnCheckChanged(int checked);

protected:
  void OnIndicatedLabelsChanged();

  QmitkLabelSelectionToolGUI();
  ~QmitkLabelSelectionToolGUI() override;

  Ui::QmitkLabelSelectionToolGUIControls* m_Controls;
  itk::SmartPointer<mitk::LabelSelectionTool> m_LabelSelectionTool;
  std::unique_ptr<QmitkFloatingTextPopup> m_FloatingPopup;

  bool m_InternalEventCall = false;
};

#endif
