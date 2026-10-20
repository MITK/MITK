/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportEditorInput_h
#define QmitkReportEditorInput_h

#include <berryIEditorInput.h>
#include <berryPlatformObject.h>

/**
 * \brief Opens a report of the QmitkReportStore in a QmitkReportEditor.
 *
 * Inputs of different reports differ, so each report gets an editor of its own.
 */
class QmitkReportEditorInput : public berry::PlatformObject, public berry::IEditorInput
{
public:
  berryObjectMacro(QmitkReportEditorInput);

  explicit QmitkReportEditorInput(int reportId);

  int GetReportId() const;

  bool Exists() const override;
  QString GetName() const override;
  QString GetToolTipText() const override;
  QIcon GetIcon() const override;
  const berry::IPersistableElement* GetPersistable() const override;
  berry::Object* GetAdapter(const QString& adapterType) const override;

  bool operator==(const berry::Object* other) const override;

private:
  int m_ReportId;
};

#endif
