/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNLayoutEditorWidget_h
#define QmitkMxNLayoutEditorWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncDimension.h>

#include <QPointer>
#include <QStringList>
#include <QWidget>

class QmitkMxNCellMapWidget;
class QmitkMultiWidgetLayoutSelectionWidget;
class QStackedWidget;
class QTableWidget;
class QToolButton;
class QVBoxLayout;

/**
 * \brief The MxN editor's authoritative layout and synchronization editor.
 *
 * One structural surface for both jobs the layout document describes: the
 * cell arrangement (grid size, presets, save/load - the embedded layout
 * selection controls) and the per-dimension synchronization groups.
 *
 * The default face is visual: an interactive cell map mirroring the live
 * layout (tiles colored by navigation group, with a per-dimension sync
 * barcode), plus one card per group (hue, editable display name, dimension
 * checkboxes, re-converge, geometry reinit). Cells join a group by selecting
 * tiles and clicking the card's assign button, or by drag and drop in either
 * direction. Joining a group that synchronizes nothing yet links the
 * navigation bundle (pan/zoom/slice/crosshair) as the common-case default.
 *
 * The power-user face - the full per-dimension matrix with offset editors,
 * the complete v3-layout link model - is deliberately tucked behind the
 * "Advanced" toggle.
 *
 * The widget drives the multi widget's public sync API directly (a peer in
 * the same module; no delegate interface until a second consumer exists) and
 * follows the engine's change signals, so it doubles as a live legend while
 * docked. The embedded layout selection controls are exposed via
 * GetLayoutSelectionWidget so the hosting view can wire their apply/save/
 * load signals to the editor part exactly like the former toolbar popup.
 *
 * UI-handler mutations are exposed as public methods so tests can drive the
 * widget against a real multi widget headlessly.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNLayoutEditorWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkMxNLayoutEditorWidget(QWidget* parent = nullptr);
  ~QmitkMxNLayoutEditorWidget() override;

  /**
   * \brief Attach to (or detach from, with nullptr) the MxN editor whose
   *        layout and sync state this widget edits.
   */
  void SetMultiWidget(QmitkMxNMultiWidget* multiWidget);
  QmitkMxNMultiWidget* GetMultiWidget() const;

  /**
   * \brief The embedded layout-shape controls (grid size, presets,
   *        save/load). The hosting view connects their signals to the
   *        editor part; they are not wired module-internally.
   */
  QmitkMultiWidgetLayoutSelectionWidget* GetLayoutSelectionWidget() const;

  /**
   * \brief Add every given cell to 'group': each cell is linked on the
   *        group's currently synchronized dimensions, or on the navigation
   *        bundle when the group synchronizes nothing yet.
   */
  void AssignCellsToGroup(const QStringList& windowIds, const std::string& group);

  /**
   * \brief Link or unlink every current member cell of 'group' on one
   *        dimension. Membership is defined by the engine's links: a no-op
   *        for a group without members.
   */
  void ApplyDimensionToGroup(const std::string& group, QmitkMxNSyncDimension dimension, bool enabled);

  /**
   * \brief Link or unlink the data-selection axis for every current member
   *        cell of 'group'. Selection is single-valued per cell, so enabling
   *        moves each member's selection group to this group (last-writer-wins
   *        if a member already had a different one); disabling returns each
   *        member to the default selection group.
   */
  void ApplySelectionToGroup(const std::string& group, bool enabled);

  /**
   * \brief Add a cell to / remove a cell from a group (see
   *        AssignCellsToGroup for the join semantics; leaving clears the
   *        cell's links to the group on every dimension).
   */
  void SetCellMembership(const QString& windowId, const std::string& group, bool member);

  /** \brief Link pan+zoom+slice+crosshair for every member cell of 'group'. */
  void LinkNavigationBundle(const std::string& group);

  /** \brief Re-converge every offset dimension (slice/zoom/pan) the group links. */
  void ReconvergeGroup(const std::string& group);

  /**
   * \brief Reinit the geometry-authority component of the group's first
   *        member cell (no-op for a group without members).
   */
  void ReinitGroupGeometry(const std::string& group);

  /** \brief Create a fresh synchronization group and return its id. */
  std::string CreateGroup();

public Q_SLOTS:

  /** \brief Coalesced full refresh from the engine state. */
  void ScheduleRebuild();

private:

  void Rebuild();
  QWidget* BuildGroupCard(const QmitkMxNMultiWidget::SyncGroupInfo& info);
  void RebuildMatrix(const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos,
                     const std::vector<QmitkMxNMultiWidget::WindowDescriptor>& descriptors);

  /** \brief All member cells of the group over every dimension, pre-order. */
  std::vector<QString> GroupMembers(const std::string& group) const;

  /** \brief Dimensions the group currently links for at least one cell. */
  std::vector<QmitkMxNSyncDimension> GroupDimensions(const std::string& group) const;

  /** \brief Whether at least one member cell's selection group is this group. */
  bool GroupSelectionEnabled(const std::string& group) const;

  static QString CellLabel(const QmitkMxNMultiWidget::WindowDescriptor& descriptor);

  QPointer<QmitkMxNMultiWidget> m_MultiWidget;

  QmitkMultiWidgetLayoutSelectionWidget* m_LayoutSelection;
  QmitkMxNCellMapWidget* m_CellMap;
  QStackedWidget* m_Faces;
  QVBoxLayout* m_GroupsLayout;    // card list inside the scrollable main face
  QTableWidget* m_Matrix;
  QToolButton* m_AddGroupButton;
  QToolButton* m_AdvancedButton;

  bool m_RebuildPending = false;

};

#endif
