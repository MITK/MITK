/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataNodeOpacityAction_h
#define QmitkDataNodeOpacityAction_h

#include <org_mitk_gui_qt_application_Export.h>

#include <QmitkAbstractDataNodeAction.h>
#include <QWidgetAction>

class QSlider;

class MITK_QT_APP QmitkDataNodeOpacityAction : public QWidgetAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private:

  void InitializeAction() override;
  void InitializeWithDataNode(const mitk::DataNode* node) override;

  void OnOpacityChanged(int value);

  std::optional<float> GetOpacity(mitk::DataNode* node);
  std::vector<mitk::DataNode*> GetSelectedOpacityNodes();

  QSlider* m_Slider;
};

#endif
