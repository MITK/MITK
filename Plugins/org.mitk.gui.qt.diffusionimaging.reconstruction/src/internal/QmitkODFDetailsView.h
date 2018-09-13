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

#ifndef _QMITKQmitkODFDetailsView_H_INCLUDED
#define _QMITKQmitkODFDetailsView_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include "mitkILifecycleAwarePart.h"

#include "ui_QmitkODFDetailsViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkOdfImage.h>
#include <vtkTransform.h>
#include <vtkDoubleArray.h>
#include <vtkOdfSource.h>
#include <vtkSmartPointer.h>
#include <QmitkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <itkDiffusionTensor3D.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkTensorImage.h>

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.
*/

class QmitkODFDetailsView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkODFDetailsView();
  virtual ~QmitkODFDetailsView();

  typedef mitk::OdfImage::ScalarPixelType TOdfPixelType;
  typedef mitk::OdfImage::PixelType       OdfVectorType;
  typedef mitk::OdfImage::ItkOdfImageType OdfVectorImgType;

  typedef mitk::TensorImage::PixelType          TensorPixelType;
  typedef mitk::TensorImage::ItkTensorImageType TensorImageType;

  virtual void CreateQtPartControl(QWidget *parent) override;


protected slots:

  void OnSliceChanged();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  virtual void SetFocus() override;

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

  void UpdateOdf(); ///< called if slice position or datamanager selection has changed

  Ui::QmitkODFDetailsViewControls*  m_Controls;

  int                               m_OdfNormalization; ///< normalization method defined in the visualization view
  mitk::DataNode::Pointer       m_ImageNode;
  QmitkSliceNavigationListener  m_SliceChangeListener;
};



#endif // _QmitkODFDetailsView_H_INCLUDED

