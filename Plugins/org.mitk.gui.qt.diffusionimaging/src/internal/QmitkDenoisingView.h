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

//

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
//template <typename mitk::DiffusionPixelType>
class QmitkDenoisingView : public QmitkFunctionality//, public mitk::DiffusionImageSource< DiffusionPixelType >
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDenoisingView();
  virtual ~QmitkDenoisingView();

/*  typedef float TOdfPixelType;
  typedef itk::Vector<TOdfPixelType,QBALL_ODFSIZE> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;

  typedef itk::DiffusionTensor3D< TOdfPixelType >  TensorPixelType;
  typedef itk::Image< TensorPixelType, 3 >         TensorImageType;*/
  typedef short DiffusionPixelType;
  typedef mitk::DiffusionImage< DiffusionPixelType > DiffusionImageType;

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  void OnSliceChanged(const itk::EventObject& e);

protected slots:

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  void UpdateOdf(); ///< called if slice position or datamanager selection has changed

  Ui::QmitkDenoisingViewControls*  m_Controls;
  QmitkStdMultiWidget*              m_MultiWidget;


  mitk::DataNode::Pointer m_ImageNode;

  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;
  int m_PropertyObserverTag;
};



#endif // _QmitkDenoisingView_H_INCLUDED
