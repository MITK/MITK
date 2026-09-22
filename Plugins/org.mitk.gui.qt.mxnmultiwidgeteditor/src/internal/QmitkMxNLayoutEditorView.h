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

#include <mitkIRenderWindowPartListener.h>

#include <QMetaObject>

#include <vector>

class QCheckBox;
class QmitkButtonOverlayWidget;
class QmitkMxNLayoutEditorWidget;

/**
 * \brief Dockable Workbench view hosting the MxN layout editor.
 *
 * Deliberately thin: all editor logic lives in the QtWidgets-module
 * QmitkMxNLayoutEditorWidget; this view only finds the active MxN editor,
 * wires its multi widget into the hosted widget, and connects the embedded
 * layout-shape controls to the editor part (the same wiring the former
 * toolbar popup had), keeping the BlueBerry dependency out of the module.
 */
class QmitkMxNLayoutEditorView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:

  /** \brief Open the MxN display, so the view has something to configure. */
  void OpenMxNDisplay();

  void DisconnectLayoutControls();

  /** \brief Ask the user before a layout change discards a non-trivial
   *         synchronization configuration; returns true to proceed. No prompt
   *         (returns true) when the current config is the trivial default. */
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

};

#endif
