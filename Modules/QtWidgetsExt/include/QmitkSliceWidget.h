/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSliceWidget_h
#define QmitkSliceWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <QmitkRenderWindow.h>
#include <mitkDataStorage.h>
#include <mitkSliceNavigationController.h>
#include <QmitkSliceNavigationWidget.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkStandaloneDataStorage.h>
#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkSliceWidgetUi;
}

/**
 * \brief Widget for displaying a single 2D slice of image data with a slice navigation slider.
 *
 * Combines a QmitkRenderWindow with a QmitkSliceNavigationWidget and an optional
 * level window widget. Supports axial, coronal, and sagittal views with a right-click
 * popup menu for switching. The widget uses a standalone data storage.
 *
 * \sa QmitkRenderWindow, QmitkSliceNavigationWidget, mitk::SliceNavigationController
 */
class MITKQTWIDGETSEXT_EXPORT QmitkSliceWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the slice widget.
   * \param[in] parent The parent widget.
   * \param[in] name The object name.
   * \param[in] f Window flags.
   */
  QmitkSliceWidget(QWidget *parent = nullptr, const char *name = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkSliceWidget() override;

  /**
   * \brief Get the VTK prop renderer used for rendering.
   * \return Pointer to the VtkPropRenderer.
   */
  mitk::VtkPropRenderer *GetRenderer();

  /**
   * \brief Get the selection frame widget.
   * \return Pointer to the QFrame.
   */
  QFrame *GetSelectionFrame();

  /** \brief Force a camera fit and render update. */
  void UpdateGL();

  /**
   * \brief Handle right-click to show the view direction popup menu.
   * \param[in] e The mouse event.
   */
  void mousePressEvent(QMouseEvent *e) override;

  /**
   * \brief Enable or disable the right-click view direction popup menu.
   * \param[in] b True to enable the popup.
   */
  void setPopUpEnabled(bool b);

  /**
   * \brief Set the data storage for this widget.
   * \param[in] storage The standalone data storage.
   */
  void SetDataStorage(mitk::StandaloneDataStorage::Pointer storage);

  /**
   * \brief Get the current data storage.
   * \return Pointer to the StandaloneDataStorage.
   */
  mitk::StandaloneDataStorage *GetDataStorage();

  /**
   * \brief Get the slice navigation widget (slider).
   * \return Pointer to the QmitkSliceNavigationWidget.
   */
  QmitkSliceNavigationWidget* GetSliceNavigationWidget();

  /**
   * \brief Check whether the level window widget is enabled.
   * \return True if the level window is enabled.
   */
  bool IsLevelWindowEnabled();

  /**
   * \brief Get the render window.
   * \return Pointer to the QmitkRenderWindow.
   */
  QmitkRenderWindow *GetRenderWindow();

  /**
   * \brief Get the slice navigation controller.
   * \return Pointer to the mitk::SliceNavigationController.
   */
  mitk::SliceNavigationController *GetSliceNavigationController() const;

  /**
   * \brief Get the camera rotation controller.
   * \return Pointer to the mitk::CameraRotationController.
   */
  mitk::CameraRotationController *GetCameraRotationController() const;

  /**
   * \brief Get the base controller.
   * \return Pointer to the mitk::BaseController.
   */
  mitk::BaseController *GetController() const;

public slots:

  /**
   * \brief Set the data node to display using the current view direction.
   * \param[in] it Iterator pointing to a data node in the data storage.
   */
  void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it);

  /**
   * \brief Set the data node to display with a specified view direction.
   * \param[in] it Iterator pointing to a data node in the data storage.
   * \param[in] view The anatomical plane (Axial, Coronal, Sagittal).
   */
  void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it, mitk::AnatomicalPlane view);

  /**
   * \brief Set the data node to display using the current view direction.
   * \param[in] node The data node containing image data.
   */
  void SetData(mitk::DataNode::Pointer node);

  /**
   * \brief Set the data node to display with a specified view direction.
   * \param[in] node The data node containing image data.
   * \param[in] view The anatomical plane.
   */
  void SetData(mitk::DataNode::Pointer node, mitk::AnatomicalPlane view);

  /**
   * \brief Initialize the widget with the given view direction.
   * \param[in] viewDirection The anatomical plane to display.
   */
  void InitWidget(mitk::AnatomicalPlane viewDirection);

  /**
   * \brief Handle mouse wheel to navigate slices.
   * \param[in] e The wheel event.
   */
  void wheelEvent(QWheelEvent *e) override;

  /**
   * \brief Change the view direction from the popup menu.
   * \param[in] val The menu action ("Axial", "Coronal", or "Sagittal").
   */
  void ChangeView(QAction *val);

  /**
   * \brief Enable or disable the level window widget.
   * \param[in] enable True to enable.
   */
  void SetLevelWindowEnabled(bool enable);

protected:
  QmitkRenderWindow *m_RenderWindow;
  mitk::AnatomicalPlane m_View;

private:
  std::unique_ptr<Ui::QmitkSliceWidgetUi> m_Controls;
  bool popUpEnabled;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::SlicedGeometry3D::Pointer m_SlicedGeometry;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;

  QMenu *popUp;
};

#endif
