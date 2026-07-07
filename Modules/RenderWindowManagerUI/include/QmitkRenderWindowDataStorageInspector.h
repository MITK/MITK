/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowDataStorageInspector_h
#define QmitkRenderWindowDataStorageInspector_h

// render window manager UI module
#include <MitkRenderWindowManagerUIExports.h>

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>
#include <QmitkRenderWindowDataStorageTreeModel.h>

// qt widgets module
#include <QmitkAbstractDataStorageInspector.h>
#include <memory>

namespace Ui { class QmitkRenderWindowDataStorageInspector; }

/**
 * \brief Inspector widget for managing data node layers and view directions in MITK render windows.
 *
 * QmitkRenderWindowDataStorageInspector provides a GUI for manipulating the data node layer
 * ordering within a specific render window, as well as switching the view direction (axial,
 * coronal, sagittal, 3D) via radio buttons. It combines a tree view backed by a
 * QmitkRenderWindowDataStorageTreeModel with radio buttons for view direction control.
 *
 * Plugins or views that use this inspector must call SetControlledRenderer() to specify
 * which render windows are managed, and SetActiveRenderWindow() to select the currently
 * displayed render window.
 *
 * The tree view supports drag-and-drop for reordering layers, extended row selection,
 * and alternating row colors.
 *
 * \sa QmitkAbstractDataStorageInspector, QmitkRenderWindowDataStorageTreeModel,
 *     QmitkRenderWindowDataStorageListModel, QmitkDataStorageLayerStackModel,
 *     mitk::RenderWindowLayerController, mitk::RenderWindowViewDirectionController
 */
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkRenderWindowDataStorageInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  /**
   * \brief Construct the inspector widget.
   *
   * Initializes the tree view, layer controller, and view direction controller.
   * Sets up drag-and-drop, selection behavior, and radio button connections.
   *
   * \param[in] parent Optional parent widget.
   */
  QmitkRenderWindowDataStorageInspector(QWidget* parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkRenderWindowDataStorageInspector() override;

  /**
   * \brief Get the underlying tree view widget.
   *
   * \return Pointer to the QAbstractItemView (QTreeView) used by this inspector.
   */
  QAbstractItemView* GetView() override;

  /**
   * \brief Get the underlying tree view widget (const version).
   *
   * \return Const pointer to the QAbstractItemView (QTreeView) used by this inspector.
   */
  const QAbstractItemView* GetView() const override;

  /**
   * \brief Set the selection mode of the tree view.
   *
   * \param[in] mode The QAbstractItemView::SelectionMode to apply to the tree view.
   */
  void SetSelectionMode(SelectionMode mode) override;

  /**
   * \brief Get the current selection mode of the tree view.
   *
   * \return The current QAbstractItemView::SelectionMode of the tree view.
   */
  SelectionMode GetSelectionMode() const override;

  /**
   * \brief Set the list of render windows controlled by this inspector.
   *
   * Forwards the renderer vector to the tree model and the view direction controller.
   *
   * \param[in] controlledRenderer Vector of base renderers to manage.
   */
  void SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer);

  /**
   * \brief Set the currently active render window by its identifier string.
   *
   * Looks up the base renderer by name, updates the tree model to display its layer
   * stack, and checks the radio button matching the renderer's default view direction.
   *
   * \param[in] renderWindowId The name / identifier of the render window to activate.
   */
  void SetActiveRenderWindow(const QString& renderWindowId);

private Q_SLOTS:

  void ModelRowsInserted(const QModelIndex& parent, int start, int end);

  void ChangeViewDirection(const QString& viewDirection);

private:

  void Initialize() override;
  void SetUpConnections();

  std::unique_ptr<Ui::QmitkRenderWindowDataStorageInspector> m_Controls;

  std::unique_ptr<QmitkRenderWindowDataStorageTreeModel> m_StorageModel;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
};

#endif
