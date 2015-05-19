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

#ifndef QmitkFiberProcessingView_h
#define QmitkFiberProcessingView_h

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberProcessingViewControls.h"

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
\brief View to process fiber bundles. Supplies methods to extract fibers from the bundle, join and subtract bundles and much more.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkFiberProcessingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    itkUCharImageType;

  static const std::string VIEW_ID;

  QmitkFiberProcessingView();
  virtual ~QmitkFiberProcessingView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
  virtual void StdMultiWidgetNotAvailable() override;
  virtual void Activated() override;

protected slots:

  void OnDrawCircle();          ///< add circle interactors etc.
  void OnDrawPolygon();         ///< add circle interactors etc.
  void GenerateAndComposite();
  void GenerateOrComposite();
  void GenerateNotComposite();

  void JoinBundles();               ///< merge selected fiber bundles
  void SubstractBundles();          ///< subtract bundle A from bundle B. Not commutative! Defined by order of selection.
  void GenerateRoiImage();          ///< generate binary image of selected planar figures.

  void Remove();
  void Extract();
  void Modify();
  void UpdateGui();     ///< update button activity etc. dpending on current datamanager selection

  virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name, const char *propertyKey = NULL, mitk::BaseProperty *property = NULL );

protected:

  void MirrorFibers();              ///< mirror bundle on the specified plane
  void ResampleSelectedBundles();   ///< smooth fiber bundle using the specified number of sampling points per cm.
  void DoImageColorCoding();        ///< color fibers by selected scalar image
  void DoCurvatureColorCoding();    ///< color fibers by curvature
  void CompressSelectedBundles();   ///< remove points below certain error threshold
  void WeightFibers();

  void RemoveWithMask(bool removeInside);
  void RemoveDir();
  void ApplyCurvatureThreshold();   ///< remove/split fibers with a too high curvature threshold
  void PruneBundle();               ///< remove too short/too long fibers

  void ExtractWithMask(bool onlyEnds, bool invert);
  void ExtractWithPlanarFigure();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;

  Ui::QmitkFiberProcessingViewControls* m_Controls;
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

  void AddCompositeToDatastorage(mitk::DataNode::Pointer pfc, std::vector<mitk::DataNode::Pointer> children, mitk::DataNode::Pointer parentNode=NULL);
  void debugPFComposition(mitk::PlanarFigureComposite::Pointer , int );
  void WritePfToImage(mitk::DataNode::Pointer node, mitk::Image* image);
  mitk::DataNode::Pointer GenerateTractDensityImage(mitk::FiberBundle::Pointer fib, bool binary, bool absolute);
  mitk::DataNode::Pointer GenerateColorHeatmap(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsImage(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsPointSet(mitk::FiberBundle::Pointer fib);

  void NodeAdded( const mitk::DataNode* node ) override;
  void NodeRemoved(const mitk::DataNode* node) override;
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

