/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToFPointSetWidget_h
#define QmitkToFPointSetWidget_h

#include <MitkToFUIExports.h>
#include "ui_QmitkToFPointSetWidgetControls.h"

//mitk headers
#include <mitkCameraIntrinsics.h>
#include <mitkDataStorage.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkPropertyList.h>
#include <mitkTextAnnotation2D.h>
#include <mitkToFDistanceImageToPointSetFilter.h>

//Qmitk headers
#include <QmitkRenderWindow.h>

// vtk includes
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkRenderer.h>

/**
* @brief Widget allowing interaction with point sets for measurement and PointSet definition
*
* The widget allows to
* 1. Measure the distance between two points in 3D ToF space by clicking the points in the 2D slices
* 2. Defining a ToF PointSet both in 2D and 3D. CameraIntrinsics are used for calculation between 2D and 3D
*
* NOTE:
* You have to make sure that the widget is initialized at a position in the plugin using it, where the distance
* image is available. CleanUp has to be called to make sure that all observers and renderers are removed correctly.
*
* @ingroup ToFUI
*/
class MITKTOFUI_EXPORT QmitkToFPointSetWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFPointSetWidget(QWidget* p = nullptr, Qt::WindowFlags f1 = nullptr);
    ~QmitkToFPointSetWidget() override;

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief initializes the widget. Observers to the change events of the point sets are created, text actors are activated
    to be rendered into the foreground of the render window.
    \param renderWindowHashMap
    \param dataStorage DataStorage to add PointSets
    \param cameraIntrinsics
    */
    void InitializeWidget(QHash<QString, QmitkRenderWindow*> renderWindowHashMap, mitk::DataStorage::Pointer dataStorage, mitk::CameraIntrinsics::Pointer cameraIntrinsics=nullptr);
    /*!
    \brief cleans up the widget when it's functionality is not used anymore.
    Removes observers and deletes foreground renderer
    */
    void CleanUpWidget();
    /*!
    \brief set the image holding the distance information used for measuring
    */
    void SetDistanceImage(mitk::Image::Pointer distanceImage);
    /*!
    \brief Set intrinsic parameters of the used device
    */
    void SetCameraIntrinsics(mitk::CameraIntrinsics::Pointer cameraIntrinsics);

  signals:

  protected slots:
    /*!
    \brief Activates the interactor for the measurement point set
    */
    void OnMeasurement();
    /*!
    \brief Activates the interactor for the point set
    */
    void OnPointSet();

  protected:

    /*!
    \brief function called when the 2D measurement PointSet has changed
    */
    void MeasurementPointSetChanged();
    /*!
    \brief function called when the 2D PointSet has changed
    */
    void PointSetChanged();

    Ui::QmitkToFPointSetWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::DataStorage::Pointer m_DataStorage; ///< member holding the set DataStorage

    mitk::Image::Pointer m_DistanceImage; ///< image holding the range data of the ToF camera
    mitk::CameraIntrinsics::Pointer m_CameraIntrinsics; ///< intrinsic parameters of the camera

    mitk::TextAnnotation2D::Pointer m_TextAnnotationAxial; ///< text annotation used to display measurements in axial window
    mitk::TextAnnotation2D::Pointer m_TextAnnotationSagittal; ///< text annotation used to display measurement in axial window
    mitk::TextAnnotation2D::Pointer m_TextAnnotationCoronal; ///< text annotation used to display measurement in axial window
    mitk::TextAnnotation2D::Pointer m_TextAnnotation3D; ///< text annotation used to display measurement in 3d window
    mitk::VtkPropRenderer::Pointer m_RendererAxial; ///< renderer of axial render window
    mitk::VtkPropRenderer::Pointer m_RendererSagittal; ///< renderer of sagittal render window
    mitk::VtkPropRenderer::Pointer m_RendererCoronal; ///< renderer of coronal render window
    mitk::VtkPropRenderer::Pointer m_Renderer3D; ///< renderer of 3D render window

    mitk::PointSet::Pointer m_MeasurementPointSet2D; ///< PointSet holding the 2D ToF image point selection used for measuring
    mitk::DataNode::Pointer m_MeasurementPointSet3DNode; ///< DataNode holding the 3D ToF coordinates used for measuring
    mitk::PointSet::Pointer m_PointSet2D; ///< PointSet holding the 2D ToF image points
    mitk::DataNode::Pointer m_PointSet3DNode; ///< DataNode holding the 3D ToF coordinates

    mitk::PointSetDataInteractor::Pointer m_PointSetInteractor; ///< PointSetInteractor used for PointSet definition
    mitk::PointSetDataInteractor::Pointer m_MeasurementPointSetInteractor; ///< PointSetInteractor used for measurement

    long m_MeasurementPointSetChangedObserverTag; ///< observer tag for measurement PointSet observer
    long m_PointSetChangedObserverTag; ///< observer tag for PointSet observer
//    long m_DistanceImageChangedObserverTag; ///< observer tag for distance image observer

    int m_WindowHeight; ///< Height of the renderWindow

  private:
};

#endif
