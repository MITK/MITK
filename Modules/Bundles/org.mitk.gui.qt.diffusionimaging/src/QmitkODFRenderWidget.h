/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkODFRenderWidget_H_
#define QmitkODFRenderWidget_H_


#include <org_mitk_gui_qt_diffusionimaging_Export.h>

#include <QWidget>
#include <QmitkRenderWindow.h>
#include <QBoxLayout>
#include <vtkPolyData.h>
#include <mitkSurface.h>
#include <mitkDataNode.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkStandaloneDataStorage.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <mitkGlobalInteraction.h>

class DIFFUSIONIMAGING_EXPORT QmitkODFRenderWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkODFRenderWidget( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  virtual ~QmitkODFRenderWidget();

  void GenerateODF( itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf );

protected:

  QHBoxLayout* QmitkODFRenderWidgetLayout;
  QmitkRenderWindow* m_RenderWindow;

  mitk::Surface::Pointer m_Surface;
  mitk::StandaloneDataStorage::Pointer m_ds;
  mitk::DataNode::Pointer m_Node;
};
#endif /*QmitkODFRenderWidget_H_*/
