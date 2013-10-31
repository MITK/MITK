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

#ifndef _QMITKQmitkDenoisingView_H_INCLUDED
#define _QMITKQmitkDenoisingView_H_INCLUDED

#include <berryISelectionListener.h>

#include <mitkDiffusionImage.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkDenoisingViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkQBallImage.h>
#include <vtkTransform.h>
#include <vtkDoubleArray.h>
#include <vtkOdfSource.h>
#include <vtkSmartPointer.h>
#include <QmitkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <itkDiffusionTensor3D.h>

#include <mitkDiffusionImage.h>
#include <itkStatisticsImageFilter.h>
#include <itkNonLocalMeansDenoisingFilter.h>

//

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkDenoisingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDenoisingView();
  virtual ~QmitkDenoisingView();

  typedef short DiffusionPixelType;
  typedef mitk::DiffusionImage< DiffusionPixelType > DiffusionImageType;
  typedef itk::NonLocalMeansDenoisingFilter<DiffusionPixelType, DiffusionPixelType> NonLocalMeansDenoisingFilterType;

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

protected slots:

  void StartDenoising();

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  void UpdateLabelText();

  Ui::QmitkDenoisingViewControls*  m_Controls;
  QmitkStdMultiWidget*              m_MultiWidget;


  mitk::DataNode::Pointer m_ImageNode;

  int m_PropertyObserverTag;
};



#endif // _QmitkDenoisingView_H_INCLUDED
