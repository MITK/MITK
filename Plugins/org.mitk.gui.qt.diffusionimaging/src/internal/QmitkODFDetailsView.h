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
#include <mitkDiffusionImage.h>
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

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.

  \sa QmitkFunctionality
  \ingroup Functionalities
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

  typedef float TOdfPixelType;
  typedef itk::Vector<TOdfPixelType,QBALL_ODFSIZE> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;

  typedef itk::DiffusionTensor3D< TOdfPixelType >  TensorPixelType;
  typedef itk::Image< TensorPixelType, 3 >         TensorImageType;

  virtual void CreateQtPartControl(QWidget *parent);

  void OnSliceChanged(const itk::EventObject& e);

protected slots:

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes );

  virtual void SetFocus();
  void Visible();
  void Hidden();
  void Activated();
  void Deactivated();

  void UpdateOdf(); ///< called if slice position or datamanager selection has changed

  Ui::QmitkODFDetailsViewControls*  m_Controls;

  /** observer flags */
  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;
  int m_PropertyObserverTag;

  /** ODF related variables like mesh structure, values etc. */
  vtkPolyData*                      m_TemplateOdf;  ///< spherical base mesh
  vtkSmartPointer<vtkTransform>     m_OdfTransform;
  vtkSmartPointer<vtkDoubleArray>   m_OdfVals;
  vtkSmartPointer<vtkOdfSource>     m_OdfSource;

  int                               m_OdfNormalization; ///< normalization method defined in the visualization view

  /** rendering of the ODF */
  vtkActor*                     m_VtkActor;
  vtkPolyDataMapper*            m_VtkMapper;
  vtkRenderer*                  m_Renderer;
  vtkRenderWindow*              m_VtkRenderWindow;
  vtkRenderWindowInteractor*    m_RenderWindowInteractor;
  vtkCamera*                    m_Camera;

  mitk::DataNode::Pointer m_ImageNode;
};



#endif // _QmitkODFDetailsView_H_INCLUDED

