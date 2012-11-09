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

#ifndef _QMITKMITKSURFACEMATERIALEDITORVIEW_H_INCLUDED
#define _QMITKMITKSURFACEMATERIALEDITORVIEW_H_INCLUDED

#include <QmitkFunctionality.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>
#include <string>

#include "ui_QmitkMITKSurfaceMaterialEditorViewControls.h"

/*
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "QtGui/QMenubarUpdatedEvent"
*/

#include "QmitkRenderWindow.h"
#include "mitkCommon.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkShaderProperty.h"
#include "mitkSurface.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"

/*!

  \brief QmitkMITKSurfaceMaterialEditorView

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkMITKSurfaceMaterialEditorView : public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkMITKSurfaceMaterialEditorView();
  QmitkMITKSurfaceMaterialEditorView(const QmitkMITKSurfaceMaterialEditorView& other)
  {
     Q_UNUSED(other)
     throw std::runtime_error("Copy constructor not implemented");
  }

  virtual ~QmitkMITKSurfaceMaterialEditorView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

protected slots:

  void SurfaceSelected();

protected:

  Ui::QmitkMITKSurfaceMaterialEditorViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

private:

  mitk::Surface::Pointer m_Surface;
  mitk::DataStorage::Pointer m_DataTree;
  mitk::DataNode::Pointer m_DataNode;
  mitk::DataNode::Pointer m_SelectedDataNode;

  std::list<std::string> fixedProperties;
  std::list<std::string> shaderProperties;

  unsigned long observerIndex;
  bool observerAllocated;
  mitk::ShaderProperty::Pointer observedProperty;

  void InitPreviewWindow();

  int usedTimer;

  void timerEvent( QTimerEvent *e );

  void RefreshPropertiesList();

  void postRefresh();

  void shaderEnumChange(const itk::Object *caller, const itk::EventObject &event);

  berry::IStructuredSelection::ConstPointer m_CurrentSelection;
  berry::ISelectionListener::Pointer m_SelectionListener;
};




#endif // _QMITKMITKSURFACEMATERIALEDITORVIEW_H_INCLUDED

