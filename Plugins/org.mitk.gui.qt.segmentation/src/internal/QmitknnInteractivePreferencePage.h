/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractivePreferencePage_h
#define QmitknnInteractivePreferencePage_h

#include <berryIQtPreferencePage.h>
#include <memory>

namespace Ui
{
  class QmitknnInteractivePreferencePage;
}

class QmitknnInteractivePreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitknnInteractivePreferencePage();
  ~QmitknnInteractivePreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private Q_SLOTS:
  void OnModelSourceToggled();
  void OnInferenceModeToggled();
  void OnBrowseLocalModelPath();

  /** \brief Refreshes the model-checkpoint combo box from nnInteractive's model
   *         management (full install only). User-initiated; falls back to the
   *         editable text when offline or before installation. */
  void OnRefreshModelsClicked();

  /** \brief Enables the torch.compile checkbox only when the computation
   *         backend selects a CUDA device, since torch.compile is meaningful
   *         only there. Connected to the backend radio buttons and the GPU
   *         device line edit. */
  void OnComputationBackendChanged();

private:
  void OnUninstallButtonClicked();
  void UpdateUninstallButton();

  void OnCheckForUpdatesButtonClicked();
  void UpdateCheckForUpdatesButton();

  /** \brief Runs the pip upgrade dialog (update mode). Returns true on success. */
  bool RunUpdate(bool clientOnly);

  std::unique_ptr<Ui::QmitknnInteractivePreferencePage> m_Ui;
  QWidget* m_Control;

  // Whether nnInteractive is installed client-only (no local inference). Set in
  // Update() from the nnInteractive/installMode preference; drives the disabling
  // of the Local inference option and the local-only setting groups.
  bool m_ClientOnly = false;
};

#endif
