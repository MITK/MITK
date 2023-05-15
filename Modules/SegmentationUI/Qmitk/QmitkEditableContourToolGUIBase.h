/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEditableContourToolGUIBase_h
#define QmitkEditableContourToolGUIBase_h

#include <QmitkToolGUI.h>
#include <MitkSegmentationUIExports.h>

class QButtonGroup;

namespace mitk
{
  class EditableContourTool;
}

namespace Ui
{
  class QmitkEditableContourToolGUIControls;
}

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::EditableContourTool based classes.
\sa mitk::LassoTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkEditableContourToolGUIBase : public QmitkToolGUI
{
  Q_OBJECT

public:
  /// \cond
  enum class MITKSEGMENTATIONUI_EXPORT Mode
  {
    Add,
    Subtract
  };
  /// \endcond

  mitkClassMacro(QmitkEditableContourToolGUIBase, QmitkToolGUI);
  itkFactorylessNewMacro(Self);

protected slots:

  void OnNewToolAssociated(mitk::Tool*);
  void OnConfirmSegmentation();
  void OnClearContour();
  void OnAutoConfirm(bool on);
  void OnModeToggled(Mode mode);
  void OnShowInformation(bool on);

protected:
  QmitkEditableContourToolGUIBase();
  ~QmitkEditableContourToolGUIBase() override;

  Ui::QmitkEditableContourToolGUIControls* m_Controls;
  QButtonGroup* m_ModeButtonGroup;
  itk::SmartPointer<mitk::EditableContourTool> m_NewTool;
};

#endif
