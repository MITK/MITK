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

  QmitkMxNLayoutEditorWidget* m_LayoutEditorWidget = nullptr;

  /** \brief Editor-wide mouse-interaction scheme toggle, live only while an MxN
   *         editor part is active. */
  QCheckBox* m_PacsSchemeBox = nullptr;

  /** Covers the editor while no MxN display is open: with nothing to configure
   *  the controls would otherwise sit there greyed out and unexplained. Carries
   *  the action that resolves it, so the display is one click away. */
  QmitkButtonOverlayWidget* m_NoDisplayOverlay = nullptr;

  /** \brief Per-attachment connections of the layout-shape controls to the
   *         active editor part; dropped on part deactivation. */
  std::vector<QMetaObject::Connection> m_LayoutConnections;

  /** \brief Tracked here: the view is bound (RenderWindowPartActivated) before
   *         BlueBerry first reports it visible. */
  bool m_Visible = false;

  /** \brief The editor currently in arrange mode on this view's behalf. */
  QPointer<QmitkMxNMultiWidget> m_ArrangedMultiWidget;

};

#endif
