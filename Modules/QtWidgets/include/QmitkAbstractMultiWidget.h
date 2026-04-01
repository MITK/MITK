/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAbstractMultiWidget_h
#define QmitkAbstractMultiWidget_h

// mitk qt widgets module
#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEventHandler.h>
#include <mitkInteractionSchemeSwitcher.h>
#include <mitkPoint.h>

// qt
#include <QWidget>

// c++
#include <map>
#include <memory>

class QmitkMultiWidgetLayoutManager;
class QmitkRenderWindow;
class QmitkRenderWindowWidget;

namespace mitk
{
  class DataStorage;
  class InteractionEventHandler;
}

/**
* \brief Abstract base widget for displaying multiple render windows at once.
*
* QmitkAbstractMultiWidget is a QWidget that can be subclassed to display multiple
* render windows at once. Render windows can dynamically be added and removed to
* change the layout of the multi widget. A subclass of this multi widget can be
* used inside a QmitkAbstractMultiWidgetEditor.
*
* The class uses the DisplayActionEventBroadcast and DisplayActionEventHandler
* classes to load a state machine and set an event configuration.
*
* Using the Synchronize() function the user can enable or disable the
* synchronization of display action events.
*
* \sa DisplayActionEventFunctions
* \sa QmitkStdMultiWidget
* \sa QmitkMxNMultiWidget
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractMultiWidget : public QWidget
{
  Q_OBJECT

public:

  /** \brief Convenience type alias for a shared pointer to a render window widget. */
  using RenderWindowWidgetPointer = std::shared_ptr<QmitkRenderWindowWidget>;
  /** \brief Convenience type alias for a map of render window widgets keyed by name. */
  using RenderWindowWidgetMap = std::map<QString, std::shared_ptr<QmitkRenderWindowWidget>>;
  /** \brief Convenience type alias for a hash of render windows keyed by name. */
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;

  QmitkAbstractMultiWidget(QWidget* parent = 0,
                           Qt::WindowFlags f = {},
                           const QString& multiWidgetName = "multiwidget");

  virtual ~QmitkAbstractMultiWidget();

  /** \brief Initialize the multi widget. Must be implemented by subclasses. */
  virtual void InitializeMultiWidget() = 0;
  /** \brief Called when the multi widget is opened. Default implementation does nothing. */
  virtual void MultiWidgetOpened() { }
  /** \brief Called when the multi widget is closed. Default implementation does nothing. */
  virtual void MultiWidgetClosed() { }

  /**
   * \brief Sets the data storage for all render window widgets.
   * \param[in] dataStorage Pointer to the data storage. May be nullptr.
   */
  virtual void SetDataStorage(mitk::DataStorage* dataStorage);
  /**
   * \brief Returns the current data storage.
   * \return Pointer to the data storage, or nullptr if none is set.
   */
  mitk::DataStorage* GetDataStorage() const;

  /** \brief Returns the number of rows in the current layout. */
  int GetRowCount() const;
  /** \brief Returns the number of columns in the current layout. */
  int GetColumnCount() const;
  /**
   * \brief Sets the layout to the given number of rows and columns.
   * \param[in] row    Number of rows.
   * \param[in] column Number of columns.
   */
  virtual void SetLayout(int row, int column);

  /** \brief Enable or disable synchronization of display action events. */
  virtual void Synchronize(bool) { };
  /**
   * \brief Sets the interaction scheme for the multi widget.
   * \param[in] scheme The interaction scheme to use.
   */
  virtual void SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

  /**
   * \brief Returns the interaction event handler.
   * \return Pointer to the interaction event handler.
   */
  mitk::InteractionEventHandler* GetInteractionEventHandler();
  /**
   * \brief Sets the display action event handler.
   * \param[in] displayActionEventHandler The event handler to use (ownership is transferred).
   */
  void SetDisplayActionEventHandler(std::unique_ptr<mitk::DisplayActionEventHandler> displayActionEventHandler);
  /**
   * \brief Returns the display action event handler.
   * \return Pointer to the display action event handler.
   */
  mitk::DisplayActionEventHandler* GetDisplayActionEventHandler();

  /** \brief Returns all render window widgets. */
  RenderWindowWidgetMap GetRenderWindowWidgets() const;
  /** \brief Returns all 2D render window widgets. */
  RenderWindowWidgetMap Get2DRenderWindowWidgets() const;
  /** \brief Returns all 3D render window widgets. */
  RenderWindowWidgetMap Get3DRenderWindowWidgets() const;
  /**
   * \brief Returns the render window widget at the given row and column.
   * \param[in] row    Row index.
   * \param[in] column Column index.
   * \return Shared pointer to the widget, or nullptr if not found.
   */
  RenderWindowWidgetPointer GetRenderWindowWidget(int row, int column) const;
  /**
   * \brief Returns the render window widget with the given name.
   * \param[in] widgetName The widget name.
   * \return Shared pointer to the widget, or nullptr if not found.
   */
  RenderWindowWidgetPointer GetRenderWindowWidget(const QString& widgetName) const;
  /**
   * \brief Returns the render window widget that owns the given render window.
   * \param[in] renderWindow The render window to search for.
   * \return Shared pointer to the widget, or nullptr if not found.
   */
  RenderWindowWidgetPointer GetRenderWindowWidget(const QmitkRenderWindow* renderWindow) const;
  /** \brief Returns a hash of all render windows keyed by name. */
  RenderWindowHash GetRenderWindows() const;
  /**
   * \brief Returns the render window at the given row and column.
   * \param[in] row    Row index.
   * \param[in] column Column index.
   * \return Pointer to the render window, or nullptr if not found.
   */
  QmitkRenderWindow* GetRenderWindow(int row, int column) const;
  /**
   * \brief Returns the render window with the given widget name.
   * \param[in] widgetName The widget name.
   * \return Pointer to the render window, or nullptr if not found.
   */
  virtual QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const;
  /**
   * \brief Returns the render window that shows the given anatomical orientation.
   * \param[in] orientation The anatomical plane orientation.
   * \return Pointer to the render window, or nullptr if not found.
   */
  virtual QmitkRenderWindow* GetRenderWindow(const mitk::AnatomicalPlane& orientation) const = 0;

  /**
   * \brief Sets the active render window widget.
   * \param[in] activeRenderWindowWidget The widget to set as active.
   */
  virtual void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget);
  /** \brief Returns the currently active render window widget. */
  RenderWindowWidgetPointer GetActiveRenderWindowWidget() const;
  /** \brief Returns the first render window widget. */
  RenderWindowWidgetPointer GetFirstRenderWindowWidget() const;
  /** \brief Returns the last render window widget. */
  RenderWindowWidgetPointer GetLastRenderWindowWidget() const;

  /** \brief Returns the name of this multi widget. */
  QString GetMultiWidgetName() const;
  /**
   * \brief Returns the widget name for the given row and column.
   * \param[in] row    Row index.
   * \param[in] column Column index.
   * \return The widget name string.
   */
  virtual QString GetNameFromIndex(int row, int column) const;
  /**
   * \brief Returns the widget name for the given linear index.
   * \param[in] index The linear index.
   * \return The widget name string.
   */
  virtual QString GetNameFromIndex(size_t index) const;

  /** \brief Returns the number of render window widgets. */
  unsigned int GetNumberOfRenderWindowWidgets() const;

  /**
   * \brief Requests an update of the render window with the given name.
   * \param[in] widgetName The widget name.
   */
  void RequestUpdate(const QString& widgetName);
  /** \brief Requests an update of all render windows. */
  void RequestUpdateAll();
  /**
   * \brief Forces an immediate update of the render window with the given name.
   * \param[in] widgetName The widget name.
   */
  void ForceImmediateUpdate(const QString& widgetName);
  /** \brief Forces an immediate update of all render windows. */
  void ForceImmediateUpdateAll();

  /**
  * \brief Initialize the render windows of the concrete multi widget to the given geometry.
  *
  * The concrete implementation is subclass-specific, no default implementation is provided here.
  *
  * \param[in] geometry      The geometry to be used to initialize / update the
  *                           render window's time and slice navigation controller.
  * \param[in] resetCamera   If true, the camera and crosshair will be reset to the default view (centered, no zoom).
  *                           If false, the current crosshair position and the camera zoom will be stored and reset
  *                           after the reference geometry has been updated.
  */
  virtual void InitializeViews(const mitk::TimeGeometry* geometry, bool resetCamera) = 0;

  /**
  * \brief Define the reference geometry for interaction within a render window.
  *
  * The concrete implementation is subclass-specific, no default implementation is provided here.
  *
  * \param[in] referenceGeometry  The interaction reference geometry for the base renderer of the concrete multi widget.
  *                                For more details, see BaseRenderer::SetInteractionReferenceGeometry.
  */
  virtual void SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry) = 0;

  /**
  * \brief Returns true if the render windows are coupled; false if not.
  *
  * Render windows are coupled if the slice navigation controllers of the render windows
  * are connected, meaning the same geometry is used for the render windows.
  */
  virtual bool HasCoupledRenderWindows() const = 0;

  /**
   * \brief Sets the selected position in the specified render window.
   * \param[in] newPosition The new 3D position.
   * \param[in] widgetName  The name of the target render window widget.
   */
  virtual void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) = 0;
  /**
   * \brief Returns the selected position in the specified render window.
   * \param[in] widgetName The name of the render window widget.
   * \return The currently selected 3D position.
   */
  virtual const mitk::Point3D GetSelectedPosition(const QString& widgetName) const = 0;

  /**
   * \brief Sets the crosshair visibility.
   * \param[in] visible True to show the crosshair, false to hide it.
   */
  virtual void SetCrosshairVisibility(bool visible) = 0;
  /** \brief Returns whether the crosshair is currently visible. */
  virtual bool GetCrosshairVisibility() const = 0;
  /**
   * \brief Sets the gap size of the crosshair.
   * \param[in] gapSize The gap size in pixels.
   */
  virtual void SetCrosshairGap(unsigned int gapSize) = 0;

  /** \brief Resets the crosshair to the default position. */
  virtual void ResetCrosshair() = 0;

  /**
   * \brief Sets the widget plane mode (e.g., rotation, swivel).
   * \param[in] mode The plane mode identifier.
   */
  virtual void SetWidgetPlaneMode(int mode) = 0;

  /**
   * \brief Activates or deactivates the render window menu widget.
   * \param[in] state True to activate, false to deactivate.
   */
  virtual void ActivateMenuWidget(bool state);
  /** \brief Returns whether the render window menu widget is enabled. */
  virtual bool IsMenuWidgetEnabled() const;

  /** \brief Returns the layout manager for this multi widget. */
  QmitkMultiWidgetLayoutManager* GetMultiWidgetLayoutManager() const;

signals:

  void ActiveRenderWindowChanged();

private slots:

  void OnFocusChanged(itk::Object*, const itk::EventObject& event);

protected:

  /**
   * \brief Adds a render window widget with the given name.
   * \param[in] widgetName          The name for the new widget.
   * \param[in] renderWindowWidget  The render window widget to add.
   */
  virtual void AddRenderWindowWidget(const QString& widgetName, RenderWindowWidgetPointer renderWindowWidget);
  /** \brief Removes the last render window widget. */
  virtual void RemoveRenderWindowWidget();

private:

  /**
  * \brief Called by SetLayout() and can be customized in subclasses.
  */
  virtual void SetLayoutImpl() = 0;
  /**
  * \brief Called by SetInteractionScheme() and can be customized in subclasses.
  */
  virtual void SetInteractionSchemeImpl() = 0;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif
