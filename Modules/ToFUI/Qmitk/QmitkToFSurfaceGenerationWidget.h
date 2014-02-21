/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _QMITKTOFSURFACEGENERATIONWIDGET_H_INCLUDED
#define _QMITKTOFSURFACEGENERATIONWIDGET_H_INCLUDED

#include "mitkTOFUIExports.h"
#include "ui_QmitkToFSurfaceGenerationWidgetControls.h"

// QT headers
#include <QWidget>
// vtk includes
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>

#include <mitkDataNode.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkToFImageGrabber.h>
#include <mitkCameraIntrinsics.h>
#include <mitkSurface.h>

class QmitkStdMultiWidget;

/** Documentation:
  *
  *
  * \ingroup ToFUI
  */
class mitkTOFUI_EXPORT QmitkToFSurfaceGenerationWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFSurfaceGenerationWidget (QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFSurfaceGenerationWidget ();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();


    mitk::ToFDistanceImageToSurfaceFilter::Pointer GetToFDistanceImageToSurfaceFilter();

    /**
     * @brief IsActive Check if the widget was initialized correctly.
     * @return True for success.
     */
    bool IsActive();

    /**
     * @brief Initialize Initialize the surface generation widget.
     * @param filter ToFDistanceImageToSurfaceFilter for surface computation.
     * @param grabber ToFImageGrabber to get/set device properties.
     * @param intrinsics Intrincs of the device.
     * @param surface Generated Surface.
     */
    void Initialize(mitk::ToFDistanceImageToSurfaceFilter::Pointer filter, mitk::ToFImageGrabber::Pointer grabber, mitk::CameraIntrinsics::Pointer intrinsics, mitk::DataNode::Pointer surface, vtkSmartPointer<vtkCamera> camera);

    bool UpdateSurface();
    mitk::Surface::Pointer GetSurface();
protected slots:

    /**
       * @brief OnCompute3DDataCheckboxChecked Slot beeing called, if the "surface"-checkbox is clicked. This method initializes the surface once, if it is necessary.
       * @param checked Is it checked or not?
       */
    void OnCompute3DDataCheckboxChecked(bool checked);
    /**
     * @brief OnShowAdvancedOptionsCheckboxChecked Show/hide advanced options.
     * @param checked show/hide
     */
    void OnShowAdvancedOptionsCheckboxChecked(bool checked);
    /*!
      \brief Slot trigged from the triangulation checkbox to decide if the mesh is triangulated or not.
      */
    void OnTriangulationCheckBoxChanged();

    /*!
      \brief Slot trigged from the triangulation threshold spin box. Changed the threshold for connecting a vertex during triangulation.
      */
    void OnTriangulationThresholdSpinBoxChanged();

    /**
     * @brief OnDistanceColorMapCheckBoxChecked Show the distance color mapping (vtkColorTransferFunction) on the surface.
     * @param checked Show/hide.
     */
    void OnDistanceColorMapCheckBoxChecked(bool checked);

    /**
     * @brief OnRGBTextureCheckBoxChecked Put the RGB image as texture on the generated surface/point cloud.
     * @param checked Show/hide texture.
     */
    void OnRGBTextureCheckBoxChecked(bool checked);


  protected:

    Ui::QmitkToFSurfaceGenerationWidgetControls* m_Controls;


  private:
    void FindReconstructionModeProperty();

    mitk::ToFDistanceImageToSurfaceFilter::Pointer m_ToFDistanceImageToSurfaceFilter;
    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber;
    mitk::CameraIntrinsics::Pointer m_CameraIntrinsics;
    mitk::DataNode::Pointer m_SurfaceNode;
    mitk::Surface::Pointer m_Surface;
    bool m_Active;
    vtkSmartPointer<vtkCamera> m_Camera3d;

};

#endif // _QMITKTOFVISUALISATIONSETTINGSWIDGET_H_INCLUDED
