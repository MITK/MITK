/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegWithPreviewToolGUIBase_h
#define QmitkSegWithPreviewToolGUIBase_h

#include "QmitkToolGUI.h"

#include "mitkSegWithPreviewTool.h"

#include <MitkSegmentationUIExports.h>

class QCheckBox;
class QPushButton;
class QBoxLayout;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI base clase for tools derived from mitk::SegWithPreviewTool.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSegWithPreviewToolGUIBase : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkSegWithPreviewToolGUIBase, QmitkToolGUI);
  itkCloneMacro(Self);

  itkGetConstMacro(Mode2D, bool);

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnAcceptPreview();

protected:
  QmitkSegWithPreviewToolGUIBase(bool mode2D);
  ~QmitkSegWithPreviewToolGUIBase() override;

  virtual void DisconnectOldTool(mitk::SegWithPreviewTool* oldTool);
  virtual void ConnectNewTool(mitk::SegWithPreviewTool* newTool);

  /**This method is called by OnNewToolAssociated if the UI is initialized the
   first time to allow derived classes to introduce own UI code. Overwrite to change.
   The implementation should ensure that alle widgets needed for the tool UI are
   properly allocated. If one needs to eecute time (e.g. to connect events between the tool
   and the UI) each time the tool changes, override the functions ConnectNewTool() and
   DisconnectOldTool().*/
  virtual void InitializeUI(QBoxLayout* mainLayout);

  void BusyStateChanged(bool isBusy) override;

  using EnableConfirmSegBtnFunctionType = std::function<bool(bool)>;
  EnableConfirmSegBtnFunctionType m_EnableConfirmSegBtnFnc;

  /**This method is used to control/set the enabled state of the tool UI
    widgets. It is e.g. used if the busy state is changed (see BusyStateChanged).
    Override the default implmentation, e.g. if a tool adds his own UI elements
    (normally by overriding InitializeUI()) and wants to control how the widgets
    are enabled/disabled.*/
  virtual void EnableWidgets(bool enabled);

  template <class TTool>
  TTool* GetConnectedToolAs()
  {
    return dynamic_cast<TTool*>(m_Tool.GetPointer());
  };

  void SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle mergeStyle);
  void SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle);

private:
  QCheckBox* m_CheckIgnoreLocks = nullptr;
  QCheckBox* m_CheckMerge = nullptr;
  QCheckBox* m_CheckProcessAll = nullptr;
  QPushButton* m_ConfirmSegBtn = nullptr;
  QBoxLayout* m_MainLayout = nullptr;

  /**Indicates if the tool is in 2D or 3D mode.*/
  bool m_Mode2D;

  mitk::SegWithPreviewTool::Pointer m_Tool;
};

#endif
