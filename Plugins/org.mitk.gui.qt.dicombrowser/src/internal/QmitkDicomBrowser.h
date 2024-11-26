/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomBrowser_h
#define QmitkDicomBrowser_h

#include "QmitkStoreSCPLauncherBuilder.h"

#include <berryIPartListener.h>
#include <berryQtEditorPart.h>

class QmitkDicomDataEventPublisher;
class QmitkDicomDirectoryListener;
class QmitkStoreSCPLauncher;
class DicomEventHandler;

namespace mitk
{
  class IPreferences;
}

namespace Ui
{
  class QmitkDicomBrowserControls;
}

class ctkFileDialog;

/**
* \brief QmitkDicomBrowser is an editor providing functionality for dicom storage and import and query retrieve functionality.
*
* \sa berry::IPartListener
* \ingroup ${plugin_target}_internal
*/
class QmitkDicomBrowser : public berry::QtEditorPart, virtual public berry::IPartListener
{
  Q_OBJECT

public:
  static const std::string EDITOR_ID;
  static const QString TEMP_DICOM_FOLDER_SUFFIX;

  berryObjectMacro(QmitkDicomBrowser)

  /**
   * \brief QmitkDicomBrowser constructor.
   */
  QmitkDicomBrowser();

  /**
   * \brief QmitkDicomBrowser destructor.
   */
  ~QmitkDicomBrowser() override;

  /**
   * \brief Init initialize the editor.
   */
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;

  void SetFocus() override;
  void DoSave() override {}
  void DoSaveAs() override {}
  bool IsDirty() const override { return false; }
  bool IsSaveAsAllowed() const override { return false; }

  void OnPreferencesChanged(const mitk::IPreferences* prefs);

signals:
  /**
   * \brief SignalStartDicomImport is enitted when dicom directory for import was selected.
   */
  void SignalStartDicomImport(const QString&);

protected:

  /// \brief Called when import is finished.
  void OnDicomImportFinished();

  /// \brief Called when Query Retrieve or Import Folder was clicked.
  void OnTabChanged(int);

  /// \brief Called when view button is clicked. Sends out an event for adding the current selected file to the mitkDataStorage.
  void OnViewButtonAddToDataManager(const QHash<QString, QVariant>& eventProperties);

  /// \brief Called when status of dicom storage provider changes.
  void OnStoreSCPStatusChanged(const QString& status);

  /// \brief Called when dicom storage provider emits a network error.
  void OnDicomNetworkError(const QString& status);

  /// \brief StartStoreSCP starts  dicom storage provider.
  void StartStoreSCP();

  /// \brief StopStoreSCP stops dicom storage provider.
  void StopStoreSCP();

  /// \brief TestHandler initializes event handler.
  void TestHandler();

  /// \brief CreateTemporaryDirectory creates temporary directory in which temporary dicom objects are stored.
  void CreateTemporaryDirectory();

  /// \brief StartDicomDirectoryListener starts dicom directory listener.
  void StartDicomDirectoryListener();

  /**
  * \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomBrowserControls.h.
  *
  * \param parent is a pointer to the parent widget
  */
  void CreateQtPartControl(QWidget *parent) override;

  /// \brief SetPluginDirectory Sets plugin directory.
  void SetPluginDirectory();

  Events::Types GetPartEventTypes() const override;

  Ui::QmitkDicomBrowserControls* m_Controls;
  ctkFileDialog* m_ImportDialog;
  QmitkDicomDirectoryListener* m_DicomDirectoryListener;
  QmitkStoreSCPLauncherBuilder m_Builder;
  QmitkStoreSCPLauncher* m_StoreSCPLauncher;
  DicomEventHandler* m_Handler;
  QmitkDicomDataEventPublisher* m_Publisher;
  QString m_PluginDirectory;
  QString m_TempDirectory;
  QString m_DatabaseDirectory;
};

#endif
