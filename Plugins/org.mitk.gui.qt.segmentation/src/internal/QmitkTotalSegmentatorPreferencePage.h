/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorPreferencePage_h
#define QmitkTotalSegmentatorPreferencePage_h

#include <berryIQtPreferencePage.h>

#include <memory>

namespace Ui
{
  class QmitkTotalSegmentatorPreferencePage;
}

/** \brief Minimal preferences for the TotalSegmentator tool.
 *
 * The environment is fully managed by MITK (installed from the tool GUI), so this
 * page only exposes what the user actually decides: the compute device, the
 * (optional) license for the non-open tasks, and maintenance actions (update /
 * uninstall). All environment access happens through subprocesses, so uninstall
 * needs no in-process module guard.
 */
class QmitkTotalSegmentatorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkTotalSegmentatorPreferencePage();
  ~QmitkTotalSegmentatorPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget *parent) override;
  QWidget *GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  /** \brief Applies the entered license key to TotalSegmentator (subprocess). */
  void OnApplyLicenseClicked();

  /** \brief Clears any TotalSegmentator license (subprocess). */
  void OnRemoveLicenseClicked();

  /** \brief Deletes all downloaded model weights (re-downloaded on demand). */
  void OnClearModelWeightsClicked();

  /** \brief Reads the license currently stored in TotalSegmentator's config,
   *         reflects it in the line edit, and syncs the cached hasLicense
   *         preference. No-op when not installed or the query fails. */
  void DetectLicense();

  /** \brief Removes the managed virtual environment. */
  void OnUninstallButtonClicked();

  /** \brief Enables/disables the license and maintenance controls and updates the
   *         status text based on whether the environment is installed. */
  void RefreshState();

  std::unique_ptr<Ui::QmitkTotalSegmentatorPreferencePage> m_Ui;
  QWidget *m_Control;
};

#endif
