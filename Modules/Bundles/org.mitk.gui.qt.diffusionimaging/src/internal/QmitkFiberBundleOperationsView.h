/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkFiberBundleOperationsView_h
#define QmitkFiberBundleOperationsView_h

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberBundleOperationsViewControls.h"

#include "mitkDataStorage.h"
#include "mitkDataStorageSelection.h"

#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "mitkPlanarFigureComposite.h"
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <itkVTKImageImport.h>
#include <itkVTKImageExport.h>
#include <vtkImageStencil.h>
#include <itkRegionOfInterestImageFilter.h>
#include <vtkSmartPointer.h>

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

struct FboSelListener;

/*!
\brief QmitkFiberBundleView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkFiberBundleOperationsView : public QmitkFunctionality
{


  friend struct FboSelListener;

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    MaskImage3DType;
  typedef itk::Image< float, 3 >            FloatImageType;

  static const std::string VIEW_ID;

  QmitkFiberBundleOperationsView();
  virtual ~QmitkFiberBundleOperationsView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void Activated();

  protected slots:

    /// \brief Called when the user clicks the GUI button
  void ActionDrawEllipseTriggered();
  void ActionDrawPolygonTriggered();
  void DoFiberExtraction();
  void generatePFCompo_AND();
  void generatePFCompo_OR();
  void generatePFCompo_NOT();

  void JoinBundles();
  void SubstractBundles();
  void GenerateROIImage();
  void GenerationStart();

  virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
                                      const char *propertyKey = NULL, mitk::BaseProperty *property = NULL );


protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkFiberBundleOperationsViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  //void Select( mitk::DataNode::Pointer node, bool clearMaskOnFirstArgNULL=false, bool clearImageOnFirstArgNULL=false );

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

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

private:

  int m_EllipseCounter;
  int m_PolygonCounter;
  //contains the selected FiberBundles
  std::vector<mitk::DataNode::Pointer> m_SelectedFB;

  //contains the selected PlanarFigures
  std::vector<mitk::DataNode::Pointer> m_SelectedPF;

  mitk::Image::ConstPointer m_Image;
  mitk::Image::Pointer m_InternalImage;
  mitk::PlanarFigure::Pointer m_PlanarFigure;
  float m_UpsamplingFactor;
  MaskImage3DType::Pointer m_InternalImageMask3D;
  MaskImage3DType::Pointer m_PlanarFigureImage;
  mitk::FiberBundle::Pointer m_FiberBundle;
  mitk::DataNode::Pointer m_FiberBundleNode;

  void addPFCompositionToDataStorage(mitk::PlanarFigureComposite::Pointer, mitk::DataNode::Pointer);
  void debugPFComposition(mitk::PlanarFigureComposite::Pointer , int );
  void CompositeExtraction(mitk::DataNode::Pointer node, mitk::Image* image);
  void GenerateTractDensityImage(bool binary);
  void GenerateColorHeatmap();
  void GenerateFiberEndingsImage();
  void GenerateFiberEndingsPointSet();
  void DWIGenerationStart();
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

