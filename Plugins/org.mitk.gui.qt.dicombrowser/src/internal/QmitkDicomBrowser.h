/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomBrowser_h
#define QmitkDicomBrowser_h

#include <berryIPartListener.h>
#include <berryQtEditorPart.h>

class QmitkDicomDataEventPublisher;
class QmitkDicomEventHandler;

namespace Ui
{
  class QmitkDicomBrowser;
}

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

  berryObjectMacro(QmitkDicomBrowser)

  QmitkDicomBrowser();
  ~QmitkDicomBrowser() override;

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;
  void SetFocus() override;
  void DoSave() override {}
  void DoSaveAs() override {}
  bool IsDirty() const override { return false; }
  bool IsSaveAsAllowed() const override { return false; }

signals:
  /**
   * \brief SignalStartDicomImport is enitted when dicom directory for import was selected.
   */
  void SignalStartDicomImport(const QString&);

protected:

  /// \brief Called when import is finished.
  void OnDicomImportFinished();

  /// \brief Called when view button is clicked. Sends out an event for adding the current selected file to the mitkDataStorage.
  void OnViewButtonAddToDataManager(const QHash<QString, QVariant>& eventProperties);

  /**
  * \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomBrowserControls.h.
  *
  * \param parent is a pointer to the parent widget
  */
  void CreateQtPartControl(QWidget *parent) override;

  Events::Types GetPartEventTypes() const override;

  Ui::QmitkDicomBrowser* m_Ui;
  std::unique_ptr<QmitkDicomEventHandler> m_Handler;
  std::unique_ptr<QmitkDicomDataEventPublisher> m_Publisher;
};

#endif
