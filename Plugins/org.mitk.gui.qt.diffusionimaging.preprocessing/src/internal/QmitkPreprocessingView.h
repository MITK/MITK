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

#ifndef _QMITKPREPROCESSINGVIEW_H_INCLUDED
#define _QMITKPREPROCESSINGVIEW_H_INCLUDED

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include "ui_QmitkPreprocessingViewControls.h"

// st includes
#include <string>

// itk includes
#include <itkImage.h>
#include <itkVectorImage.h>

// mitk includes
#include <mitkImage.h>
#include "itkDWIVoxelFunctor.h"
#include <mitkDiffusionPropertyHelper.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <mitkPeakImage.h>
#include <mitkImageToItk.h>

typedef short DiffusionPixelType;

struct PrpSelListener;

/*!
 * \ingroup org_mitk_gui_qt_preprocessing_internal
 *
 * \brief Viewing and modifying diffusion weighted images (gradient reduction, resampling, b-value projection, ...)
 *
 */
class QmitkPreprocessingView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  friend struct PrpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
  typedef itk::VectorImage< short, 3 >                                    ItkDwiType;
  typedef itk::Image< unsigned char, 3 >                                  UcharImageType;
  typedef itk::Image< double, 3 >                                         ItkDoubleImageType;
  typedef mitk::DiffusionPropertyHelper                                   PropHelper;
  typedef mitk::GradientDirectionsProperty                                GradProp;

  QmitkPreprocessingView();
  virtual ~QmitkPreprocessingView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  virtual void Activated() override;

  virtual void Deactivated() override;

  virtual void Visible() override;

  virtual void Hidden() override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  static const int nrconvkernels;

protected slots:

  void AverageGradients();
  void ExtractB0();
  void MergeDwis();
  void DoApplyHeader();
  void DoReduceGradientDirections();
  void DoShowGradientDirections();
  void DoHalfSphereGradientDirections();
  void UpdateDwiBValueMapRounder(int i);
  void DoLengthCorrection();
  void DoDwiNormalization();
  void DoProjectSignal();
  void DoExtractBrainMask();
  void DoResampleImage();
  void DoCropImage();
  void DoUpdateInterpolationGui(int i);
  void DoRemoveGradient();
  void DoExtractGradient();
  void DoFlipAxis();
  void OnImageSelectionChanged();
  void DoFlipGradientDirections();
  void DoClearRotationOfGradients();
  void DoAlignImages();

protected:

  void DoADCFit();
  void DoAKCFit();
  void DoBiExpFit();
  void DoADCAverage();

  template < typename TPixel, unsigned int VImageDimension >
  void TemplatedFlipAxis( itk::Image<TPixel, VImageDimension>* itkImage);

  template < typename TPixel, unsigned int VImageDimension >
  void TemplatedCropImage( itk::Image<TPixel, VImageDimension>* itkImage);

  template < typename TPixel, unsigned int VImageDimension >
  void TemplatedResampleImage( itk::Image<TPixel, VImageDimension>* itkImage);

  /** Called by ExtractB0 if check-box activated, extracts all b0 images without averaging */
  void DoExtractBOWithoutAveraging();

  void UpdateBValueTableWidget(int);
  void UpdateGradientDetails();

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkPreprocessingViewControls* m_Controls;

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);
  void CallMultishellToSingleShellFilter( itk::DWIVoxelFunctor * functor,
                                          mitk::Image::Pointer ImPtr, QString imageName,
                                          mitk::DataNode* parent );

  void CleanBValueTableWidget();

};




#endif // _QMITKPREPROCESSINGVIEW_H_INCLUDED

