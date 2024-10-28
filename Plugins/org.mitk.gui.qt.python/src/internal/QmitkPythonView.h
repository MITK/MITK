/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonView_h
#define QmitkPythonView_h

#include "mitkPythonContext.h"
#include "ui_QmitkPythonViewControls.h"
#include <QmitkAbstractView.h>

///
/// \brief New python view (CONSOLE)
///
class QmitkPythonView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkPythonView();
  ~QmitkPythonView() = default;

protected:
  void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

protected slots:

  void OnExecuteBtnClicked();
  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnSitePackageSelected(const QString &sitePackagesFolder);

private:
  Ui::QmitkPythonViewControls *m_Controls;
  mitk::NodePredicateBase::Pointer m_ReferencePredicate;
  mitk::PythonContext::Pointer m_PythonContext;
};

#endif
