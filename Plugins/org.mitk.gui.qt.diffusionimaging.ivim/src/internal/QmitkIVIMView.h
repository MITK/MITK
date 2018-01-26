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

#ifndef _QMITKIVIMVIEW_H_INCLUDED
#define _QMITKIVIMVIEW_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkIVIMViewControls.h"

#include "itkVectorImage.h"
#include "itkImage.h"

#include <mitkDiffusionPropertyHelper.h>
#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h"
#include "itkDiffusionKurtosisReconstructionImageFilter.h"
#include <QmitkSliceNavigationListener.h>

/*!
  \brief QmitkIVIMView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.
*/

class QmitkIVIMView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkIVIMView();
  virtual ~QmitkIVIMView();

  typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType DirContainerType;
  typedef itk::DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<short, float> IVIMFilterType;
  typedef itk::DiffusionKurtosisReconstructionImageFilter<short, float> KurtosisFilterType;
  typedef itk::VectorImage<short,3> VecImgType;
  typedef itk::Image<float,3> OutImgType;

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  void OutputToDatastorage(mitk::DataNode::Pointer node);
  bool FittIVIM(itk::VectorImage<short,3>* vecimg, DirContainerType* dirs, float bval, bool multivoxel, OutImgType::IndexType &crosspos);

  void Activated() override;
  void Deactivated() override;
  void Visible() override;
  void Hidden() override;

protected slots:

  void OnSliceChanged();

  /// \brief Called when the user clicks the GUI button
  void FittIVIMStart();
  void AutoThreshold();

  void MethodCombo(int val);
  void Checkbox();

  void DStarSlider(int val);
  void BThreshSlider(int val);
  void S0ThreshSlider(int val);
  void NumItsSlider(int val);
  void LambdaSlider(int val);
  void ChooseMethod();
  void ClipboardStatisticsButtonClicked();
  void ClipboardCurveButtonClicked();

  void OnKurtosisParamsChanged();
  void UpdateGui();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  bool FitKurtosis( itk::VectorImage<short, 3> *vecimg, DirContainerType *dirs, float bval, OutImgType::IndexType &crosspos);

  Ui::QmitkIVIMViewControls* m_Controls;

  OutImgType::Pointer m_DStarMap;
  OutImgType::Pointer m_DMap;
  OutImgType::Pointer m_fMap;

  IVIMFilterType::IVIMSnapshot m_Snap;
  KurtosisFilterType::KurtosisSnapshot m_KurtosisSnap;

  bool m_Active;
  bool m_Visible;

  bool m_HoldUpdate;
  QmitkSliceNavigationListener  m_SliceChangeListener;

};



#endif // _QMITKIVIMVIEW_H_INCLUDED

