/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportEditorInput.h"
#include "QmitkReportsPluginActivator.h"
#include "QmitkReportStore.h"

#include <QIcon>

QmitkReportEditorInput::QmitkReportEditorInput(int reportId)
  : m_ReportId(reportId)
{
}

int QmitkReportEditorInput::GetReportId() const
{
  return m_ReportId;
}

bool QmitkReportEditorInput::Exists() const
{
  const auto* store = QmitkReportsPluginActivator::GetStore();
  return store != nullptr && store->Get(m_ReportId) != nullptr;
}

QString QmitkReportEditorInput::GetName() const
{
  const auto* store = QmitkReportsPluginActivator::GetStore();
  const auto* report = store != nullptr ? store->Get(m_ReportId) : nullptr;

  return report != nullptr ? report->Title : QStringLiteral("Report");
}

QString QmitkReportEditorInput::GetToolTipText() const
{
  return this->GetName();
}

QIcon QmitkReportEditorInput::GetIcon() const
{
  return QIcon();
}

const berry::IPersistableElement* QmitkReportEditorInput::GetPersistable() const
{
  return nullptr;
}

berry::Object* QmitkReportEditorInput::GetAdapter(const QString& adapterType) const
{
  return PlatformObject::GetAdapter(adapterType);
}

bool QmitkReportEditorInput::operator==(const berry::Object* other) const
{
  const auto* input = dynamic_cast<const QmitkReportEditorInput*>(other);
  return input != nullptr && input->m_ReportId == m_ReportId;
}
