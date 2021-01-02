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
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qboxlayout.h>

#include "mitkAutoSegmentationWithPreviewTool.h"

#include <MitkSegmentationUIExports.h>

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

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnAcceptPreview();

protected:
  QmitkAutoSegmentationToolGUIBase();
  ~QmitkAutoSegmentationToolGUIBase() override;

  virtual void DisconnectOldTool(mitk::AutoSegmentationWithPreviewTool* oldTool);
  virtual void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool);
  virtual void InitializeUI(QBoxLayout* mainLayout);

  void BusyStateChanged(bool isBusy) override;

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

  mitk::AutoSegmentationWithPreviewTool::Pointer m_Tool;
};

#endif
