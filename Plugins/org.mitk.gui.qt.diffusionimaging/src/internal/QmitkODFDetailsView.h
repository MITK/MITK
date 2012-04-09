/*=========================================================================

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKQmitkODFDetailsView_H_INCLUDED
#define _QMITKQmitkODFDetailsView_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

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
  \brief QmitkODFDetailsView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/

class QmitkODFDetailsView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkODFDetailsView();
  QmitkODFDetailsView(const QmitkODFDetailsView& other)
    {
      Q_UNUSED(other)
      throw std::runtime_error("Copy constructor not implemented");
    }
  virtual ~QmitkODFDetailsView();

  typedef float TOdfPixelType;
  typedef itk::Vector<TOdfPixelType,QBALL_ODFSIZE> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;

  typedef itk::DiffusionTensor3D< TOdfPixelType >  TensorPixelType;
  typedef itk::Image< TensorPixelType, 3 >         TensorImageType;

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  void OnSliceChanged(const itk::EventObject& e);

protected slots:

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed

  Ui::QmitkODFDetailsViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;

  vtkPolyData* m_TemplateOdf;
  vtkSmartPointer<vtkTransform> m_OdfTransform;
  vtkSmartPointer<vtkDoubleArray> m_OdfVals;
  vtkSmartPointer<vtkOdfSource> m_OdfSource;

  vtkActor* m_VtkActor;
  vtkPolyDataMapper* m_VtkMapper;
  vtkRenderer* m_Renderer;
  vtkRenderWindow* m_VtkRenderWindow;
  vtkRenderWindowInteractor* m_RenderWindowInteractor;
  vtkCamera* m_Camera;
  std::vector<double> m_Values;
  int m_OdfNormalization;
};



#endif // _QmitkODFDetailsView_H_INCLUDED

