/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPERSPECTIVESPREFERENCEPAGE_H
#define BERRYPERSPECTIVESPREFERENCEPAGE_H

#include <berryIQtPreferencePage.h>

namespace Ui {
class PerspectivesPreferencePage;
}

namespace berry {

class PerspectiveDescriptor;
class PerspectiveRegistry;

class PerspectivesPreferencePage : public QObject, public IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  PerspectivesPreferencePage();
  ~PerspectivesPreferencePage() override;

  void Init(IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:

  Q_SLOT void OpenPerspInSameWindow();
  Q_SLOT void OpenPerspInNewWindow();

  Q_SLOT void PerspectiveSelectionChanged();

  Q_SLOT void RevertPerspective();
  Q_SLOT void DeletePerspective();
  Q_SLOT void MakeDefaultPerspective();

  void UpdateButtons();
  void UpdatePerspectivesTable();
  void NewPerspectivesTableItem(const SmartPointer<PerspectiveDescriptor>& desc);
  bool FindOpenInstance(const SmartPointer<PerspectiveDescriptor>& desc);

  SmartPointer<PerspectiveDescriptor> GetSelectedPerspective() const;

  Ui::PerspectivesPreferencePage* ui;
  QWidget* pageWidget;

  int openPerspMode;
  SmartPointer<IPreferences> preferences;
  IWorkbench* workbench;
  PerspectiveRegistry* perspRegistry;

  QString defaultPerspectiveId;

  QList<SmartPointer<PerspectiveDescriptor> > perspectives;
  QList<SmartPointer<PerspectiveDescriptor> > perspToRevert;
  QList<SmartPointer<PerspectiveDescriptor> > perspToDelete;
};

}

#endif // BERRYPERSPECTIVESPREFERENCEPAGE_H
