/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonEnvironmentsView_h
#define QmitkPythonEnvironmentsView_h

#include <QmitkAbstractView.h>

#include <QString>
#include <QThreadPool>

#include <memory>

namespace Ui
{
  class QmitkPythonEnvironmentsView;
}

class QmitkPythonEnvironmentsView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkPythonEnvironmentsView(QObject* parent = nullptr);
  ~QmitkPythonEnvironmentsView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:
  void DeleteSelectedVirtualEnvs();
  void RefreshVirtualEnvsTreeWidget();
  void PopulateVirtualEnvsTreeWidget();
  void CalculateAllVirtualEnvSizes();
  void SetVirtualEnvSize(const QString& virtualEnvPath, quint64 totalSize, int generation);

  std::unique_ptr<Ui::QmitkPythonEnvironmentsView> m_Ui;
  QThreadPool m_SizeThreadPool;
  int m_RefreshGeneration = 0;
};

#endif
