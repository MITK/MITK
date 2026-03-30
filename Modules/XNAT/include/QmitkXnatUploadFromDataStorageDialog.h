/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatUploadFromDataStorageDialog_h
#define QmitkXnatUploadFromDataStorageDialog_h

#include <QDialog>

#include <MitkXNATExports.h>
#include <mitkDataNode.h>
#include <memory>

namespace Ui
{
  class QmitkXnatUploadFromDataStorageDialog;
}

namespace mitk
{
  class DataStorage;
}

/**
 * \brief Dialog for selecting a data node from the MITK data storage for upload to XNAT.
 *
 * QmitkXnatUploadFromDataStorageDialog presents a data node selection combo box
 * (filtered to exclude helper and hidden objects) that allows the user to choose which
 * data node from the current MITK data storage to upload to an XNAT server. The dialog
 * provides Upload and Cancel buttons.
 *
 * \note The "Upload MITK Project" checkbox and project file name entry are present in
 *       the UI but are not yet implemented.
 *
 * \sa QmitkSelectXnatUploadDestinationDialog, mitk::DataStorage, mitk::DataNode
 */
class MITKXNAT_EXPORT QmitkXnatUploadFromDataStorageDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the upload dialog.
   *
   * Sets up the UI with a data node selection combo box filtered to hide helper
   * and hidden objects. The Upload button is initially disabled until a data node
   * is selected.
   *
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkXnatUploadFromDataStorageDialog(QWidget *parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkXnatUploadFromDataStorageDialog() override;

  /**
   * \brief Set the data storage to populate the node selection combo box.
   *
   * \param[in] ds The MITK data storage containing the nodes available for upload.
   */
  void SetDataStorage(mitk::DataStorage *ds);

  /**
   * \brief Retrieve the data node selected by the user for upload.
   *
   * \return Smart pointer to the selected mitk::DataNode, or \c nullptr if no
   *         node was selected.
   */
  mitk::DataNode::Pointer GetSelectedNode();

protected slots:

  /**
   * \brief Slot called when the Upload button is clicked. Accepts the dialog.
   */
  void OnUpload();

  /**
   * \brief Slot called when the "Upload MITK Project" checkbox is toggled.
   *
   * \note Not yet implemented.
   */
  void OnUploadSceneChecked();

  /**
   * \brief Slot called when the Cancel button is clicked. Rejects the dialog.
   */
  void OnCancel();

  /**
   * \brief Slot called when the user enters a MITK project file name.
   *
   * \param[in] text The entered file name text.
   *
   * \note Not yet implemented.
   */
  void OnMITKProjectFileNameEntered(const QString &text);

  /**
   * \brief Slot called when a data node is selected in the combo box.
   *
   * Stores the selected node and enables the Upload button.
   *
   * \param[in] node The selected data node.
   */
  void OnDataSelected(const mitk::DataNode *node);

private:
  std::unique_ptr<Ui::QmitkXnatUploadFromDataStorageDialog> ui;

  mitk::DataNode::Pointer m_SelectedNode;
};

#endif
