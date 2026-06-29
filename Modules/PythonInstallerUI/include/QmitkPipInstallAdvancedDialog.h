/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPipInstallAdvancedDialog_h
#define QmitkPipInstallAdvancedDialog_h

#include <MitkPythonInstallerUIExports.h>

#include <mitkPipPackageInfo.h>

#include <QDialog>
#include <vector>

class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QVBoxLayout;

/** \brief Dialog for editing a PipInstallSpec before installation.
 *
 * Presents editable widgets for every field of the install spec, including
 * the ability to add and remove install groups and post-install steps.
 * The spec can also be loaded from and saved to JSON files.
 *
 * \sa QmitkPipInstallDialog, mitk::PipInstallSpec
 */
class MITKPYTHONINSTALLERUI_EXPORT QmitkPipInstallAdvancedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkPipInstallAdvancedDialog(const mitk::PipInstallSpec& spec, QWidget* parent = nullptr);

  /** \brief Return the edited install spec. Only meaningful after Accepted. */
  mitk::PipInstallSpec GetInstallSpec() const;

private slots:
  void OnAddGroup();
  void OnRemoveGroup();
  void OnAddStep();
  void OnRemoveStep();
  void OnLoadSpec();
  void OnSaveSpec();

private:
  struct GroupWidgets
  {
    QPlainTextEdit* requirements;
    QLineEdit* indexUrl;
    QLineEdit* extraPipArgs;
  };

  struct StepWidgets
  {
    QLineEdit* displayName;
    QPlainTextEdit* pythonCode;
    QCheckBox* optionalCheckBox;
  };

  QGroupBox* CreateGroupWidget(const mitk::PipInstallGroup& group, int index);
  QGroupBox* CreateStepWidget(const mitk::PostInstallStep& step, int index);
  void RenumberGroupTitles();
  void RenumberStepTitles();
  void RebuildFromSpec(const mitk::PipInstallSpec& spec);

  QLineEdit* m_NameEdit = nullptr;
  QLineEdit* m_VenvNameEdit = nullptr;
  QCheckBox* m_UpgradePipFirstCheckBox = nullptr;

  QVBoxLayout* m_GroupsLayout = nullptr;
  QVBoxLayout* m_StepsLayout = nullptr;

  std::vector<GroupWidgets> m_GroupWidgets;
  std::vector<QGroupBox*> m_GroupBoxes;

  std::vector<StepWidgets> m_StepWidgets;
  std::vector<QGroupBox*> m_StepBoxes;
};

#endif
