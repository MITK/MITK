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

#ifndef QmitkFiberQuantificationView_h
#define QmitkFiberQuantificationView_h

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberQuantificationViewControls.h"

#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundle.h>
#include <mitkSurface.h>

#include <itkCastImageFilter.h>
#include <itkVTKImageImport.h>
#include <itkVTKImageExport.h>
#include <itkRegionOfInterestImageFilter.h>

#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkExtractSelectedThresholds.h>
#include <vtkFloatArray.h>

/*!
\brief View to process fiber bundles. Supplies methods to generate images from the selected bundle and much more.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkFiberQuantificationView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    itkUCharImageType;

  static const std::string VIEW_ID;

  QmitkFiberQuantificationView();
  virtual ~QmitkFiberQuantificationView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
  virtual void StdMultiWidgetNotAvailable() override;
  virtual void Activated() override;

protected slots:

  void ProcessSelectedBundles();    ///< start selected operation on fiber bundle (e.g. tract density image generation)
  void CalculateFiberDirections();  ///< Calculate main fiber directions from tractogram

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;

  Ui::QmitkFiberQuantificationViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  /** Connection from VTK to ITK */
  template <typename VTK_Exporter, typename ITK_Importer>
      void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer importer)
  {
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
  }

  template <typename ITK_Exporter, typename VTK_Importer>
      void ConnectPipelines(ITK_Exporter exporter, VTK_Importer* importer)
  {
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
  }

  template < typename TPixel, unsigned int VImageDimension >
      void InternalCalculateMaskFromPlanarFigure(
          itk::Image< TPixel, VImageDimension > *image, unsigned int axis, std::string nodeName );

  template < typename TPixel, unsigned int VImageDimension >
      void InternalReorientImagePlane(
          const itk::Image< TPixel, VImageDimension > *image, mitk::Geometry3D* planegeo3D, int additionalIndex );

  void GenerateStats(); ///< generate statistics of selected fiber bundles
  void UpdateGui();     ///< update button activity etc. dpending on current datamanager selection

  std::vector<mitk::DataNode::Pointer>  m_SelectedFB;       ///< selected fiber bundle nodes
  mitk::Image::Pointer                  m_SelectedImage;
  float                                 m_UpsamplingFactor; ///< upsampling factor for all image generations
  std::vector<mitk::Surface::Pointer>   m_SelectedSurfaces;

  mitk::DataNode::Pointer GenerateTractDensityImage(mitk::FiberBundle::Pointer fib, bool binary, bool absolute);
  mitk::DataNode::Pointer GenerateColorHeatmap(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsImage(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsPointSet(mitk::FiberBundle::Pointer fib);
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

