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

#include <MitkToFUIExports.h>
#include "ui_QmitkToFSurfaceGenerationWidgetControls.h"

// QT headers
#include <QWidget>

// vtk includes
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>

//MITK
#include <mitkDataNode.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkToFImageGrabber.h>
#include <mitkCameraIntrinsics.h>
#include <mitkSurface.h>

class QmitkStdMultiWidget;

/** Documentation:
  *
  * This widget provides GUI access for all basic surface generation properties and can
  * be reused in any other GUI.
  * \ingroup ToFUI
  */
class MitkToFUI_EXPORT QmitkToFSurfaceGenerationWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFSurfaceGenerationWidget (QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFSurfaceGenerationWidget ();

    /* @brief Automatically called method. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief Automatically called method. Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();


    /**
     * @brief GetToFDistanceImageToSurfaceFilter Get the internally used surface generation filter.
     * @return ToFDistanceImageToSurfaceFilter as filter.
     */
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
     * @param generateSurface Check the generate surface combo box.
     * @param showAdvancedOptions Show/Hide advanced options.
     */
    void Initialize(mitk::ToFDistanceImageToSurfaceFilter::Pointer filter, mitk::ToFImageGrabber::Pointer grabber, mitk::CameraIntrinsics::Pointer intrinsics,
      mitk::DataNode::Pointer surface, vtkSmartPointer<vtkCamera> camera, bool generateSurface = false, bool showAdvancedOptions = true);

    /**
     * @brief UpdateSurface Generate new surface data according to the device properties
     * @return True for success.
     */
    bool UpdateSurface();

    /**
     * @brief GetSurface Get the generated surface.
     * @return Surface.
     */
    mitk::Surface::Pointer GetSurface();

protected slots:
    /**
     * @brief OnRepresentationChanged Change the representation of the surface. In other words: disable/enable
     * triangulation (Point cloud/surface). If triangulation is enabled, this will also allow for editing a
     * threshold for triangulating vertices.
     */
    void OnRepresentationChanged(int index);
    /**
     * @brief OnReconstructionChanged Change the reconstruction mode of the ToFDistanceImageToSurfaceFilter.
     */
    void OnReconstructionChanged(int index);

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
