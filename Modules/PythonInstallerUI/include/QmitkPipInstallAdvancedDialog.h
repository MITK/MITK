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
 * requirements, index URL, and extra pip arguments. If the spec carries any
 * Hugging Face downloads, an additional section per download lets the user
 * edit the repository ID and allow patterns.
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

  struct DownloadWidgets
  {
    QLineEdit* repoId;
    QPlainTextEdit* allowPatterns;
    QCheckBox* optionalCheckBox;
  };

  mitk::PipInstallSpec m_Spec;
  std::vector<GroupWidgets> m_GroupWidgets;
  std::vector<DownloadWidgets> m_DownloadWidgets;
  QCheckBox* m_UpgradePipFirstCheckBox = nullptr;
};

#endif
