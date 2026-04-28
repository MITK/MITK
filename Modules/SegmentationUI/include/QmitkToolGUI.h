/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToolGUI_h
#define QmitkToolGUI_h

#include <MitkSegmentationUIExports.h>

#include <mitkCommon.h>
#include <mitkTool.h>

#include <QWidget>

class QmitkMultiLabelInspector;

/**
  \brief Base class for GUIs belonging to mitk::Tool classes.

  \ingroup org_mitk_gui_qt_interactivesegmentation

  Created through ITK object factory.
*/

class MITKSEGMENTATIONUI_EXPORT QmitkToolGUI : public QWidget, public itk::Object
{
  Q_OBJECT

public:
  mitkClassMacroItkParent(QmitkToolGUI, itk::Object);

  /** \brief Associates the given tool with this GUI, emitting NewToolAssociated. */
  void SetTool(mitk::Tool *tool);

  /** \brief Provides the segmentation view's label inspector to this GUI.
   *
   * Set by QmitkToolSelectionBox right after SetTool(). Tool GUIs that need
   * to interact with the host's label-management UI (e.g. to create labels
   * honoring the user's naming preferences) can read it via
   * GetMultiLabelInspector(). May be null if the GUI is hosted outside a
   * segmentation view; callers must handle that case.
   */
  void SetMultiLabelInspector(QmitkMultiLabelInspector *inspector);

  /** \brief Returns the inspector associated with this GUI, or null if none. */
  QmitkMultiLabelInspector *GetMultiLabelInspector() const;

  /** \brief Intentional no-op; prevents ITK reference counting from interfering with Qt ownership. */
  void Register() const override;
  /** \brief Intentional no-op; prevents ITK reference counting from interfering with Qt ownership. */
  void UnRegister() const ITK_NOEXCEPT ITK_OVERRIDE;
  /** \brief Intentional no-op; prevents ITK reference counting from interfering with Qt ownership. */
  void SetReferenceCount(int) override;

  ~QmitkToolGUI() override;

signals:
  /** \brief Emitted when a new tool is associated with this GUI via SetTool(). */
  void NewToolAssociated(mitk::Tool *);

protected:
  QmitkToolGUI() = default;

  mitk::Tool::Pointer m_Tool;
  QmitkMultiLabelInspector *m_MultiLabelInspector = nullptr;

  /** \brief Called when the tool's busy state changes. Override to enable/disable UI elements. */
  virtual void BusyStateChanged(bool){};
};

#endif
