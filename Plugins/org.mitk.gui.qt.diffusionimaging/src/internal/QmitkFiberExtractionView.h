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

#ifndef QmitkFiberExtractionView_h
#define QmitkFiberExtractionView_h

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberExtractionViewControls.h"

#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundleX.h>
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
\brief View to process fiber bundles. Supplies methods to extract fibers from the bundle, join and subtract bundles and much more.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkFiberExtractionView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    itkUCharImageType;

  static const std::string VIEW_ID;

  QmitkFiberExtractionView();
  virtual ~QmitkFiberExtractionView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void Activated();

protected slots:

  void OnDrawCircle();          ///< add circle interactors etc.
  void OnDrawPolygon();         ///< add circle interactors etc.
  void DoFiberExtraction();     ///< Extract fibers from selected bundle
  void GenerateAndComposite();
  void GenerateOrComposite();
  void GenerateNotComposite();
  void DoRemoveOutsideMask();
  void DoRemoveInsideMask();
  void JoinBundles();               ///< merge selected fiber bundles
  void SubstractBundles();          ///< subtract bundle A from bundle B. Not commutative! Defined by order of selection.
  void GenerateRoiImage();          ///< generate binary image of selected planar figures.
  void ExtractPassingMask();        ///< extract all fibers passing the selected segmentation
  void ExtractNotPassingMask();     ///< extract all fibers NOT passing the selected segmentation
  void ExtractEndingInMask();       ///< extract all fibers passing the selected segmentation

  virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name, const char *propertyKey = NULL, mitk::BaseProperty *property = NULL );

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkFiberExtractionViewControls* m_Controls;
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
        const itk::Image< TPixel, VImageDimension > *image, mitk::BaseGeometry* planegeo3D, int additionalIndex );

  void UpdateGui();     ///< update button activity etc. dpending on current datamanager selection

  int m_CircleCounter;                                      ///< used for data node naming
  int m_PolygonCounter;                                     ///< used for data node naming
  std::vector<mitk::DataNode::Pointer>  m_SelectedFB;       ///< selected fiber bundle nodes
  std::vector<mitk::DataNode::Pointer>  m_SelectedPF;       ///< selected planar figure nodes
  std::vector<mitk::Surface::Pointer>   m_SelectedSurfaces;
  mitk::Image::Pointer                  m_SelectedImage;
  mitk::Image::Pointer                  m_InternalImage;
  mitk::PlanarFigure::Pointer           m_PlanarFigure;
  itkUCharImageType::Pointer            m_InternalImageMask3D;
  itkUCharImageType::Pointer            m_PlanarFigureImage;
  float                                 m_UpsamplingFactor; ///< upsampling factor for all image generations
  mitk::DataNode::Pointer               m_MaskImageNode;

  void AddCompositeToDatastorage(mitk::PlanarFigureComposite::Pointer pfc, mitk::DataNode::Pointer parentNode=NULL);
  void debugPFComposition(mitk::PlanarFigureComposite::Pointer , int );
  void WritePfToImage(mitk::DataNode::Pointer node, mitk::Image* image);
  mitk::DataNode::Pointer GenerateTractDensityImage(mitk::FiberBundleX::Pointer fib, bool binary, bool absolute);
  mitk::DataNode::Pointer GenerateColorHeatmap(mitk::FiberBundleX::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsImage(mitk::FiberBundleX::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsPointSet(mitk::FiberBundleX::Pointer fib);

  void NodeAdded( const mitk::DataNode* node );
  void NodeRemoved(const mitk::DataNode* node);
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

