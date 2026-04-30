/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSelectXnatUploadDestinationDialog_h
#define QmitkSelectXnatUploadDestinationDialog_h

#include <MitkXNATExports.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkSelectXnatUploadDestinationDialog;
}

class ctkXnatObject;
class ctkXnatSession;
class QModelIndex;
class QmitkXnatTreeModel;

/**
 * \brief Dialog for selecting an upload destination on an XNAT server.
 *
 * QmitkSelectXnatUploadDestinationDialog provides a user interface for choosing where to
 * upload data on an XNAT server. The user can either select an existing resource from a
 * combo box, create a new resource folder by entering a name, or browse the XNAT hierarchy
 * using a tree view to pick a destination.
 *
 * The dialog populates a tree model from the given XNAT session and offers radio buttons
 * to toggle between resource selection mode and tree-based browsing mode.
 *
 * \sa QmitkXnatTreeModel, QmitkXnatUploadFromDataStorageDialog, ctkXnatSession
 */
class MITKXNAT_EXPORT QmitkSelectXnatUploadDestinationDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the upload destination dialog.
   *
   * Initializes the tree model from the given XNAT session and populates the resource
   * combo box with the available resource names. If no resources are available, the
   * dialog defaults to tree view selection mode.
   *
   * \param[in] session The active XNAT session to browse.
   * \param[in] availableResources List of existing resource folder names to offer for selection.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkSelectXnatUploadDestinationDialog(ctkXnatSession *session, const QStringList &availableResources, QWidget *parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkSelectXnatUploadDestinationDialog() override;

  /**
   * \brief Retrieve the selected upload destination as an XNAT object.
   *
   * If the user selected or created a resource via the combo box, a new ctkXnatResource
   * is created (and saved to the server if it does not exist yet) under the object
   * identified by the resource folder URL. If the user selected a node from the tree view,
   * the corresponding ctkXnatObject is returned directly.
   *
   * \return Pointer to the ctkXnatObject representing the chosen upload destination.
   * \pre The dialog has been accepted (i.e., the user clicked Upload).
   */
  ctkXnatObject *GetUploadDestination();

  /**
   * \brief Set the XNAT resource folder URL used when creating a new resource.
   *
   * This URL identifies the parent XNAT object under which a new resource folder
   * will be created if the user opts to create one via the combo box.
   *
   * \param[in] url The XNAT REST URL of the resource folder parent.
   */
  void SetXnatResourceFolderUrl(const QString &url);

protected slots:

  /**
   * \brief Slot called when the user clicks the Upload button. Accepts the dialog.
   */
  void OnUpload();

  /**
   * \brief Slot called when the "Select Resource" radio button is toggled.
   *
   * \param[in] selectResource \c true if the resource combo box mode is selected.
   */
  void OnSelectResource(bool selectResource);

  /**
   * \brief Slot called when the "Select From Tree View" radio button is toggled.
   *
   * \param[in] selectFromTreeView \c true if the tree view browsing mode is selected.
   */
  void OnSelectFromTreeView(bool selectFromTreeView);

  /**
   * \brief Slot called when the user types a new resource folder name.
   *
   * \param[in] resourceEntered The text entered by the user as a new resource folder name.
   */
  void OnResourceEntered(const QString &resourceEntered);

  /**
   * \brief Slot called when an existing resource is selected from the combo box.
   *
   * \param[in] resource The name of the selected resource, or "Create new resource folder..."
   *            to switch to name entry mode.
   */
  void OnResourceSelected(const QString &resource);

  /**
   * \brief Slot called when an XNAT node is selected in the tree view.
   *
   * Enables the Upload button only if the selected node is a ctkXnatResource.
   *
   * \param[in] index The model index of the selected tree node.
   */
  void OnXnatNodeSelected(const QModelIndex &index);

  /**
   * \brief Slot called when the user clicks the Cancel button. Rejects the dialog.
   */
  void OnCancel();

private:
  QmitkXnatTreeModel *m_TreeModel;
  QString m_Url;
  QString m_ResourceName;
  bool m_CreateNewFolder;
  std::unique_ptr<Ui::QmitkSelectXnatUploadDestinationDialog> ui;
};

#endif
