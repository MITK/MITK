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

#ifndef QmitkODFRenderWidget_H_
#define QmitkODFRenderWidget_H_


#include <org_mitk_gui_qt_diffusionimaging_reconstruction_Export.h>

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

class DIFFUSIONIMAGING_RECONSTRUCTION_EXPORT QmitkODFRenderWidget : public QWidget
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
