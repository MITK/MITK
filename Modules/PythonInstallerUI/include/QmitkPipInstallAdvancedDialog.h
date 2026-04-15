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
class QLineEdit;
class QPlainTextEdit;

/** \brief Dialog for editing pip install groups before installation.
 *
 * Presents one section per PipInstallGroup with editable fields for
 * requirements, index URL, and extra pip arguments.
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

private:
  struct GroupWidgets
  {
    QPlainTextEdit* requirements;
    QLineEdit* indexUrl;
    QLineEdit* extraPipArgs;
  };

  mitk::PipInstallSpec m_Spec;
  std::vector<GroupWidgets> m_GroupWidgets;
  QCheckBox* m_UpgradePipFirstCheckBox = nullptr;
};

#endif
