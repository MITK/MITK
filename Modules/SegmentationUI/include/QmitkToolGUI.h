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

  /** \brief Called when the tool's busy state changes. Override to enable/disable UI elements. */
  virtual void BusyStateChanged(bool){};
};

#endif
