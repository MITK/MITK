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
#include <QmitkMxNSyncDimension.h>
#include <mitkRenderWindowViewDirectionController.h>

#include <QColor>
#include <QList>

// qt
#include <QWidget>

namespace mitk
{
  class DataStorage;
}

class QmitkRenderWindow;
class QMenu;
class QPaintEvent;
class QToolButton;

/**
* \brief Utility widget that extends a QmitkRenderWindowWidget with window-specific controls.
*
* It hosts the cell's strip controls and applies the cell's view direction to
* its renderer (SetViewDirectionSelection); the plane itself is chosen on the
* cell overlay. Slice scrolling lives in the cell's viewport navigator, not
* here. In addition, it contains
* a QmitkSynchronizedNodeSelectionWidget that controls renderer-specific
* properties and shown nodes; the cell's data-selection group (shared with
* other render windows) is stored on that widget and edited from the layout
* editor, not here.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowUtilityWidget : public QWidget
{
  Q_OBJECT

public:

  /**
  * \throws mitk::Exception if 'renderWindow' or 'dataStorage' is null.
  */
  QmitkRenderWindowUtilityWidget(
    QWidget* parent,
    QmitkRenderWindow* renderWindow,
    mitk::DataStorage* dataStorage
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
  * \brief Open the data selection popup at a global position, independent of
  *        whether the strip is revealed; the path for the keyboard and the
  *        context menu.
  */
  void ShowDataSelection(const QPoint& globalPosition);

  /**
  * \brief Apply the cell's view direction to its renderer. The source cell's
  *        plane-label picker (via MxN::SetViewDirection), an orientation-group
  *        relay (via MxN::PropagateOrientation) and a cell joining an
  *        orientation group (via MxN::SetSyncLink) funnel through here. Only 'AnatomicalPlane::Axial' / 'Coronal' /
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
  * \brief Mirror the editor-wide crosshair visibility into this cell's toggle
  *        button without re-emitting 'CrosshairToggled'.
  */
  void SetCrosshairChecked(bool visible);

  /**
  * \brief Mirror whether this cell is the maximized one into its toggle button
  *        without re-emitting 'MaximizeToggled'.
  */
  void SetMaximizeChecked(bool maximized);

  /**
  * \brief Set the sync barcode for this cell: one slot per axis (glyph, hue,
  *        tooltip; an invalid color is an unsynced gap). Pushed by the owning
  *        multi widget from the group registry.
  */
  void SetSyncBarcodeSlots(const QList<QmitkMxNSyncBarcodeWidget::AxisSlot>& axisSlots);

  /** \brief The slots the sync barcode currently shows. */
  QList<QmitkMxNSyncBarcodeWidget::AxisSlot> GetSyncBarcodeSlots() const;

Q_SIGNALS:

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
  * \brief Emitted when the user toggles the crosshair from this cell. Like
  *        clean view the state is editor-wide; the owning multi widget applies
  *        it and mirrors it back via 'SetCrosshairChecked'.
  */
  void CrosshairToggled(bool visible);

  /**
  * \brief Emitted when the user maximizes this cell, or restores the grid from
  *        it. Unlike the other toggles this one is per cell; the owning multi
  *        widget resolves which cell asked and mirrors the result back to every
  *        strip via 'SetMaximizeChecked'.
  */
  void MaximizeToggled(bool maximized);

  /**
  * \brief Emitted when the user asks for the editor-wide layout editor from
  *        this cell's sync barcode; the owning multi widget relays it to
  *        whoever hosts the view.
  */
  void LayoutEditorRequested();

  /**
  * \brief Where the pointer is on this cell's sync barcode: on the strip at all,
  *        and which axis glyph it is over (none between glyphs). The owning
  *        multi widget turns this into the editor-wide sync peek; the strip
  *        itself only reports.
  */
  void SyncPeekHovered(bool overStrip, std::optional<QmitkMxNSyncAxis> axis);

  /**
  * \brief Emitted while a popup owned by this strip (currently the data
  *        selection) is open. The popup's pointer grab reads to the cell as the
  *        pointer leaving, so the owning multi widget holds the furniture
  *        revealed for as long as this is true.
  */
  void PopupVisibilityChanged(bool visible);

protected:

  /** \brief Paints the translucent rounded backing behind the controls. */
  void paintEvent(QPaintEvent* event) override;

private:

  /** \brief Point the maximize button's icon at what the next click will do. */
  void UpdateMaximizeIcon();

  /** \brief Show either the furniture clean view hides, or the bare frame it
   *         leaves behind, according to the current state. */
  void UpdateCleanViewIcon();

  mitk::BaseRenderer* m_BaseRenderer;
  QmitkSynchronizedNodeSelectionWidget* m_NodeSelectionWidget;
  QMenu* m_DataMenu;
  QToolButton* m_CleanViewButton;
  QToolButton* m_NavigatorToggleButton;
  QToolButton* m_CrosshairButton;
  QToolButton* m_MaximizeButton;
  QmitkMxNSyncBarcodeWidget* m_SyncBarcode;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;

};

#endif
