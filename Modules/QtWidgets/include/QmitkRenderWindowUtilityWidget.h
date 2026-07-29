/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowUtilityWidget_h
#define QmitkRenderWindowUtilityWidget_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>

#include <QColor>
#include <QList>

// qt
#include <QWidget>
#include <QHBoxLayout>

namespace mitk
{
  class DataStorage;
}

class QmitkRenderWindow;
class QPaintEvent;
class QToolButton;

/**
* \brief Utility widget that extends a QmitkRenderWindowWidget with window-specific controls.
*
* It offers to select the viewing direction of the window (slice scrolling
* lives in the cell's viewport navigator, not here). In addition, it contains
* a QmitkSynchronizedNodeSelectionWidget that controls renderer-specific
* properties and shown nodes; the cell's data-selection group (shared with
* other render windows) is stored on that widget and edited from the layout
* editor, not here.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowUtilityWidget : public QWidget
{
	Q_OBJECT

public:

  QmitkRenderWindowUtilityWidget(
    QWidget* parent = nullptr,
    QmitkRenderWindow* renderWindow = nullptr,
    mitk::DataStorage* dataStorage = nullptr
  );

  ~QmitkRenderWindowUtilityWidget() override;

  using GroupSyncIndexType = int;

  /**
  * \brief The cell's data-selection group index, read from the authoritative
  *        node selection widget (returns -1 while the cell is unassigned).
  *        Read by serialization (MakeWindowDescriptor) and the sync barcode.
  */
  GroupSyncIndexType GetSyncGroup() const;

  QmitkSynchronizedNodeSelectionWidget* GetNodeSelectionWidget() const;

  /**
  * \brief Apply the cell's view direction to its renderer. Both the source
  *        cell's plane-label picker (via MxN::SetViewDirection) and an
  *        orientation-group relay (via MxN::PropagateOrientation) funnel
  *        through here. Only 'AnatomicalPlane::Axial' / 'Coronal' /
  *        'Sagittal' are supported; other planes are ignored.
  */
  void SetViewDirectionSelection(mitk::AnatomicalPlane viewDirection);

public Q_SLOTS:

  /**
  * \brief Mirror the editor-wide clean-view state into this cell's toggle
  *        button without re-emitting 'CleanViewToggled'.
  */
  void SetCleanViewChecked(bool checked);

  /**
  * \brief Mirror the editor-wide navigator mode into this cell's toggle
  *        button without re-emitting 'NavigatorToggled'.
  */
  void SetNavigatorChecked(bool expanded);

  /**
  * \brief Set the sync barcode for this cell: one slot per axis (glyph, hue,
  *        tooltip; an invalid color is an unsynced gap). Pushed by the owning
  *        multi widget from the group registry.
  */
  void SetSyncBarcodeSlots(const QList<QmitkMxNSyncBarcodeWidget::AxisSlot>& axisSlots);

Q_SIGNALS:

  void SynchronizationToggled(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);
  void SetDataSelection(const QList<mitk::DataNode::Pointer>& newSelection);

  /**
  * \brief Emitted when the user toggles clean-view mode in this cell. The
  *        mode is editor-wide; the owning multi widget applies it to every
  *        cell and mirrors it back via 'SetCleanViewChecked'.
  */
  void CleanViewToggled(bool cleanView);

  /**
  * \brief Emitted when the user toggles the navigator mode in this cell. The
  *        mode is editor-wide; the owning multi widget applies it to every
  *        cell and mirrors it back via 'SetNavigatorChecked'.
  */
  void NavigatorToggled(bool expanded);

  /**
  * \brief Emitted when the user asks for the editor-wide layout editor from
  *        this cell's "Sync" button; the owning multi widget relays it to
  *        whoever hosts the view.
  */
  void LayoutEditorRequested();

protected:

  /** \brief Paints the translucent rounded backing behind the controls. */
  void paintEvent(QPaintEvent* event) override;

private:

  mitk::BaseRenderer* m_BaseRenderer;
  QmitkSynchronizedNodeSelectionWidget* m_NodeSelectionWidget;
  QToolButton* m_CleanViewButton;
  QToolButton* m_NavigatorToggleButton;
  QmitkMxNSyncBarcodeWidget* m_SyncBarcode;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;

};

#endif
