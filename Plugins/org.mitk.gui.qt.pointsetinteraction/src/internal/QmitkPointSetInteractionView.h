/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#if !defined(QmitkPointSetInteraction_H__INCLUDED)
#define QmitkPointSetInteraction_H__INCLUDED

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>
#include <mitkWeakPointer.h>
#include <mitkDataNode.h>

namespace Ui
{
class QmitkPointSetInteractionControls;
};

/*!
\brief QmitkPointSetInteractionView
*/
class QmitkPointSetInteractionView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkPointSetInteractionView(QObject *parent=nullptr);
  ~QmitkPointSetInteractionView() override;


  void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  void SetFocus() override;

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  void NodeChanged(const mitk::DataNode* node) override;
  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
protected slots:
  void OnAddPointSetClicked();
protected:
  Ui::QmitkPointSetInteractionControls * m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedPointSetNode;
};
#endif // !defined(QmitkPointSetInteraction_H__INCLUDED)
