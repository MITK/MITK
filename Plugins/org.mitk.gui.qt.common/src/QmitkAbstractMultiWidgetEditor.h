/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAbstractMultiWidgetEditor_h
#define QmitkAbstractMultiWidgetEditor_h

#include <org_mitk_gui_qt_common_Export.h>

// org mitk gui qt common plugin
#include <QmitkAbstractRenderEditor.h>

// mitk core
#include <mitkInteractionSchemeSwitcher.h>

// berry
#include <berryIPartListener.h>

// c++
#include <memory>

class QmitkAbstractMultiWidget;
class QmitkLevelWindowWidget;

class MITK_QT_COMMON QmitkAbstractMultiWidgetEditor : public QmitkAbstractRenderEditor, public berry::IPartListener
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkAbstractMultiWidgetEditor, QmitkAbstractRenderEditor, IPartListener);

  static const QString EDITOR_ID;

  QmitkAbstractMultiWidgetEditor();
  virtual ~QmitkAbstractMultiWidgetEditor() override;

  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindow(const mitk::AnatomicalPlane& orientation) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  void InitializeViews(const mitk::TimeGeometry* geometry, bool resetCamera) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  void SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  bool HasCoupledRenderWindows() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual bool IsDecorationEnabled(const QString& decoration) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  virtual QStringList GetDecorations() const override;
  /**
  * @brief Retrieve a QmitkRenderWindow by its index.
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindowByIndex(int index) const;
  /**
  * @brief Retrieve a QmitkRenderWindow by the row and column position.
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindowByIndex(int row, int column) const;
  /**
  * @brief Set the current multi widget of this editor.
  */
  virtual void SetMultiWidget(QmitkAbstractMultiWidget* multiWidget);
  /**
  * @brief Return the current multi widget of this editor.
  */
  virtual QmitkAbstractMultiWidget* GetMultiWidget() const;
  /**
  * @brief Return the number of rows of the underlying multi widget.
  */
  virtual int GetRowCount() const;
  /**
  * @brief Return the number of columns of the underlying multi widget.
  */
  virtual int GetColumnCount() const;

  virtual QmitkLevelWindowWidget* GetLevelWindowWidget() const = 0;

public Q_SLOTS:
  /**
  * @brief A slot that can be called if the layout has been changed.
  *        This function will call the function 'SetLayout' of the multi widget where
  *        custom behavior can be implemented.
  *        Finally 'FirePropertyChange' is called to inform the workbench about an input change.
  */
  virtual void OnLayoutSet(int row, int column);
  virtual void OnSynchronize(bool synchronized);
  virtual void OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

private:

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif
