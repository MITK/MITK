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

#ifndef _QMITKIVIMVIEW_H_INCLUDED
#define _QMITKIVIMVIEW_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkIVIMViewControls.h"

#include "itkVectorImage.h"
#include "itkImage.h"

#include "mitkDiffusionImage.h"
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

  typedef mitk::DiffusionImage<short>::GradientDirectionContainerType DirContainerType;
  typedef itk::DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<short, short> IVIMFilterType;
  typedef itk::Image<short,3> OutImgType;

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  void OnSliceChanged(const itk::EventObject& e);
  void OutputToDatastorage(std::vector<mitk::DataNode*> nodes);
  void FittIVIM(itk::VectorImage<short,3>* vecimg, DirContainerType* dirs, float bval, bool multivoxel, OutImgType::IndexType &crosspos);

  void Activated();
  void Deactivated();

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
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkIVIMViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;

  OutImgType::Pointer m_DStarMap;
  OutImgType::Pointer m_DMap;
  OutImgType::Pointer m_fMap;

  IVIMFilterType::IVIMSnapshot m_Snap;

  bool m_Active;
};



#endif // _QMITKIVIMVIEW_H_INCLUDED

