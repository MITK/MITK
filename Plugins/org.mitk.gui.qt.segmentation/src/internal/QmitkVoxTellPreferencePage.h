/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVoxTellPreferencePage_h
#define QmitkVoxTellPreferencePage_h

#include <berryIQtPreferencePage.h>

#include <memory>

namespace Ui
{
  class QmitkVoxTellPreferencePage;
}

/** \brief Minimal preferences for the VoxTell tool.
 *
 * The environment is fully managed by MITK (installed from the tool GUI), so this
 * page only exposes what the user actually decides: the compute device, where
 * the model comes from, updating, and uninstalling.
 */
class QmitkVoxTellPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkVoxTellPreferencePage();
  ~QmitkVoxTellPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget *parent) override;
  QWidget *GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  /** \brief Lets the user choose the folder of a local model. */
  void OnBrowseButtonClicked();

  /** \brief Offers an update if a newer supported VoxTell is available. */
  void OnCheckForUpdatesButtonClicked();

  /** \brief Removes the managed virtual environment. */
  void OnUninstallButtonClicked();

  /** \brief Enables the path of a local model only if one is used. */
  void OnModelSourceChanged();

  /** \brief Enables the maintenance controls and sets the status text based on
   *         whether the environment is installed. */
  void RefreshState();

  std::unique_ptr<Ui::QmitkVoxTellPreferencePage> m_Ui;
  QWidget *m_Control;
};

#endif
