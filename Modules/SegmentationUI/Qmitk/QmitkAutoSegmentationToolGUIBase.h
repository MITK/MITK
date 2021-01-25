/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAutoSegmentationToolGUIBase_h_Included
#define QmitkAutoSegmentationToolGUIBase_h_Included

#include "QmitkToolGUI.h"

#include "mitkAutoSegmentationWithPreviewTool.h"

#include <MitkSegmentationUIExports.h>

class QCheckBox;
class QPushButton;
class QBoxLayout;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI base clase for tools derived from mitk::AutoSegmentationTool.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkAutoSegmentationToolGUIBase : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkAutoSegmentationToolGUIBase, QmitkToolGUI);
  itkCloneMacro(Self);

  itkGetConstMacro(Mode2D, bool);

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnAcceptPreview();

protected:
  QmitkAutoSegmentationToolGUIBase(bool mode2D);
  ~QmitkAutoSegmentationToolGUIBase() override;

  virtual void DisconnectOldTool(mitk::AutoSegmentationWithPreviewTool* oldTool);
  virtual void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool);
  virtual void InitializeUI(QBoxLayout* mainLayout);

  void BusyStateChanged(bool isBusy) override;

  using EnableConfirmSegBtnFunctionType = std::function<bool(bool)>;
  EnableConfirmSegBtnFunctionType m_EnableConfirmSegBtnFnc;

  virtual void EnableWidgets(bool enabled);

  template <class TTool>
  TTool* GetConnectedToolAs()
  {
    return dynamic_cast<TTool*>(m_Tool.GetPointer());
  };



private:
  QCheckBox* m_CheckProcessAll = nullptr;
  QCheckBox* m_CheckCreateNew = nullptr;
  QPushButton* m_ConfirmSegBtn = nullptr;
  QBoxLayout* m_MainLayout = nullptr;

  /**Indicates if the tool is in 2D or 3D mode.*/
  bool m_Mode2D;

  mitk::AutoSegmentationWithPreviewTool::Pointer m_Tool;
};

#endif
