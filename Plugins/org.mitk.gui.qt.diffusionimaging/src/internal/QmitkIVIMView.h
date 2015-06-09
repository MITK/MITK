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

#include <QmitkFunctionality.h>

#include "ui_QmitkIVIMViewControls.h"

#include "itkVectorImage.h"
#include "itkImage.h"

#include <mitkDiffusionPropertyHelper.h>
#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h"

/*!
  \brief QmitkIVIMView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/

class QmitkIVIMView : public QmitkFunctionality
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
  typedef itk::VectorImage<short,3> VecImgType;
  typedef itk::Image<float,3> OutImgType;

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
  virtual void StdMultiWidgetNotAvailable() override;

  void OnSliceChanged(const itk::EventObject& e);
  void OutputToDatastorage(std::vector<mitk::DataNode*> nodes);
  bool FittIVIM(itk::VectorImage<short,3>* vecimg, DirContainerType* dirs, float bval, bool multivoxel, OutImgType::IndexType &crosspos);

  void Activated() override;
  void Deactivated() override;

protected slots:

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

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;

  Ui::QmitkIVIMViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;

  OutImgType::Pointer m_DStarMap;
  OutImgType::Pointer m_DMap;
  OutImgType::Pointer m_fMap;

  IVIMFilterType::IVIMSnapshot m_Snap;

  mitk::DataNode::Pointer m_DiffusionImageNode;
  mitk::DataNode::Pointer m_MaskImageNode;

  bool m_Active;
};



#endif // _QMITKIVIMVIEW_H_INCLUDED

