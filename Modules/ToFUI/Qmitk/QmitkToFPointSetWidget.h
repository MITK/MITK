/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QmitkToFPointSetWidget_H_INCLUDED
#define _QmitkToFPointSetWidget_H_INCLUDED

#include "mitkTOFUIExports.h"
#include "ui_QmitkToFPointSetWidgetControls.h"

//mitk headers
#include <mitkCameraIntrinsics.h>
#include <mitkDataStorage.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>
#include <mitkPropertyList.h>
#include <mitkToFDistanceImageToPointSetFilter.h>

// Qmitk headers
#include <QmitkStdMultiWidget.h>

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
* @ingroup ToFUI
*/
class mitkTOFUI_EXPORT QmitkToFPointSetWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFPointSetWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFPointSetWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief initializes the widget
    \param stdMultiWidget QmitkStdMultiWidget used for painting overlays for measurement
    \param dataStorage DataStorage to add PointSets
    \param distanceImage range image used to calculate 3D PointSet from 2D index
    */
    void InitializeWidget(QmitkStdMultiWidget* stdMultiWidget, mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer distanceImage);

    /*!
    \brief specify the intrinsic parameters of the camera (holds focal length, principal point, distortion coefficients)
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

    QmitkStdMultiWidget* m_MultiWidget; ///< multi widget used for overlay visualization

    mitk::Image::Pointer m_DistanceImage; ///< image holding the range data of the ToF camera
    mitk::CameraIntrinsics::Pointer m_CameraIntrinsics; ///< intrinsic parameters of the camera

    vtkSmartPointer<vtkTextActor> m_VtkTextActor; ///< actor containing the text of the overlay
    vtkSmartPointer<vtkRenderer> m_ForegroundRenderer1; ///< renderer responsible for text rendering in the foreground of widget 1
    vtkSmartPointer<vtkRenderer> m_ForegroundRenderer2; ///< renderer responsible for text rendering in the foreground of widget 2
    vtkSmartPointer<vtkRenderer> m_ForegroundRenderer3; ///< renderer responsible for text rendering in the foreground of widget 3
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow1; ///< vtk render window used for showing overlay in widget 1
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow2; ///< vtk render window used for showing overlay in widget 2
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow3; ///< vtk render window used for showing overlay in widget 3

    mitk::PointSet::Pointer m_MeasurementPointSet2D; ///< PointSet holding the 2D ToF image point selection used for measuring
    mitk::DataNode::Pointer m_MeasurementPointSet3DNode; ///< DataNode holding the 3D ToF coordinates used for measuring
    mitk::PointSet::Pointer m_PointSet2D; ///< PointSet holding the 2D ToF image points
    mitk::DataNode::Pointer m_PointSet3DNode; ///< DataNode holding the 3D ToF coordinates

    mitk::PointSetInteractor::Pointer m_PointSetInteractor; ///< PointSetInteractor used for PointSet definition
    mitk::PointSetInteractor::Pointer m_MeasurementPointSetInteractor; ///< PointSetInteractor used for measurement

    long m_MeasurementPointSetChangedObserverTag; ///< observer tag for measurement PointSet observer
    long m_PointSetChangedObserverTag; ///< observer tag for PointSet observer

  private:
};

#endif // _QmitkToFPointSetWidget_H_INCLUDED
