/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNLayoutEditorView_h
#define QmitkMxNLayoutEditorView_h

#include <QmitkAbstractView.h>

#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

#include <QMetaObject>
#include <QPointer>

#include <vector>

class QCheckBox;
class QLabel;
class QmitkButtonOverlayWidget;
class QmitkMxNLayoutEditorWidget;
class QmitkMxNMultiWidget;

/**
 * \brief Dockable Workbench view hosting the MxN layout editor.
 *
 * Deliberately thin: all editor logic lives in the QtWidgets-module
 * QmitkMxNLayoutEditorWidget; this view only finds the active MxN editor,
 * wires its multi widget into the hosted widget, and connects the embedded
 * layout-shape controls to the editor part, keeping the BlueBerry dependency
 * out of the module.
 *
 * While the view is visible, the editor it is bound to is in arrange mode: the
 * render windows' peek plates stay up for selecting cells and assigning them
 * to groups. Following the view's visibility rather than a toggle of its own
 * keeps it one concept - the view is how the user arranges.
 */
class QmitkMxNLayoutEditorView : public QmitkAbstractView,
                                 public mitk::IRenderWindowPartListener,
                                 public mitk::ILifecycleAwarePart
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ~QmitkMxNLayoutEditorView() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void Activated() override;
  void Deactivated() override;
  void Visible() override;
  void Hidden() override;

protected:

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:

  /** \brief Open the MxN display, so the view has something to configure. */
  void OpenMxNDisplay();

  void DisconnectLayoutControls();

  /** \brief Put the bound editor in arrange mode exactly while the view is
   *         visible, and take a previously arranged one out of it. */
  void UpdateArrangeMode();

  /** \brief Ask the user before a layout change that rebuilds every window
   *         discards a non-trivial synchronization configuration; returns true
   *         to proceed. No prompt (returns true) when the current configuration
   *         is the trivial default. */
  bool ConfirmDestructiveLayoutChange();

  /** \brief Whether 'multiWidget' is this view's to configure: an MxN widget
   *         whose plugin has not locked its structure. */
  static bool IsBindable(const QmitkMxNMultiWidget* multiWidget);

  /** \brief After a modal or a deferred step: whether 'multiWidget' is still
   *         the bound widget, so a change confirmed before can still go to it. */
  bool IsStillBoundTo(const QmitkMxNMultiWidget* multiWidget) const;

  /** \brief Why the view is not bound to the shown part, or that it is. */
  enum class Binding
  {
    Bound,
    NoDisplay,
    Locked,
    CannotBeShown
  };

  /** \brief Shows the overlay that explains why nothing is bound, or hides it. */
  void ShowUnboundOverlay(Binding binding);

  QmitkMxNLayoutEditorWidget* m_LayoutEditorWidget = nullptr;

  /** \brief Editor-wide mouse-interaction scheme toggle, live only while the
   *         view is bound to an MxN editor. */
  QCheckBox* m_PacsSchemeBox = nullptr;

  /** \brief Names the display the view is bound to. The view follows
   *         visibility, not focus, so with several displays on screen the
   *         focused one need not be the one being configured. */
  QLabel* m_BoundDisplayLabel = nullptr;

  /** Covers the editor while the view is unbound - no MxN display visible, or only
   *  one whose plugin locked its structure or that shows an error panel: the controls would otherwise sit
   *  there greyed out and unexplained. Says which case applies and carries the
   *  action that resolves it, so a bindable display is one click away. */
  QmitkButtonOverlayWidget* m_NoDisplayOverlay = nullptr;

  /** \brief Per-attachment connections of the layout-shape controls to the
   *         active editor part; dropped on part deactivation. */
  std::vector<QMetaObject::Connection> m_LayoutConnections;

  /** \brief Tracked here: the view is bound (RenderWindowPartActivated) before
   *         BlueBerry first reports it visible. */
  bool m_Visible = false;

  /** \brief The editor currently in arrange mode on this view's behalf. */
  QPointer<QmitkMxNMultiWidget> m_ArrangedMultiWidget;

  /** \brief The part the view shows, bound or locked; null for none. Also kept
   *         for an editor with an invalid declaration, so that closing it
   *         resets the text. Kept so a lock change re-runs the binding
   *         decision on it, and so its closing returns the view to the
   *         no-display state. */
  mitk::IRenderWindowPart* m_ShownPart = nullptr;

};

#endif
