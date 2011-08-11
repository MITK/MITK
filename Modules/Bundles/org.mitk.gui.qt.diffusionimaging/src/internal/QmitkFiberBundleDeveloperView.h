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

#ifndef QmitkFiberBundleDeveloperView_h
#define QmitkFiberBundleDeveloperView_h

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberBundleDeveloperViewControls.h"

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



/*!
\brief QmitkFiberBundleView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkFiberBundleDeveloperView : public QmitkFunctionality
{


  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    MaskImage3DType;
  typedef itk::Image< float, 3 >            FloatImageType;

  static const std::string VIEW_ID;

  QmitkFiberBundleDeveloperView();
  virtual ~QmitkFiberBundleDeveloperView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void Activated();

  protected slots:

  
protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkFiberBundleDeveloperViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  //void Select( mitk::DataNode::Pointer node, bool clearMaskOnFirstArgNULL=false, bool clearImageOnFirstArgNULL=false );

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


};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

