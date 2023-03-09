/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMITKSurfaceMaterialEditorView_h
#define QmitkMITKSurfaceMaterialEditorView_h

#include <QmitkAbstractView.h>
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
#include "mitkSurface.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"

/*!

  \brief QmitkMITKSurfaceMaterialEditorView
*/
class QmitkMITKSurfaceMaterialEditorView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkMITKSurfaceMaterialEditorView();
  ~QmitkMITKSurfaceMaterialEditorView() override;

  void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  void SetFocus() override;

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

protected slots:

  void SurfaceSelected();

protected:

  Ui::QmitkMITKSurfaceMaterialEditorViewControls* m_Controls;

private:

  mitk::Surface::Pointer m_Surface;
  mitk::DataStorage::Pointer m_DataTree;
  mitk::DataNode::Pointer m_DataNode;
  mitk::DataNode::Pointer m_SelectedDataNode;

  std::list<std::string> fixedProperties;
  std::list<std::string> shaderProperties;

  unsigned long observerIndex;
  bool observerAllocated;

  void InitPreviewWindow();

  int usedTimer;

  void timerEvent( QTimerEvent *e ) override;

  void RefreshPropertiesList();

  void postRefresh();

  void shaderEnumChange(const itk::Object *caller, const itk::EventObject &event);

  berry::IStructuredSelection::ConstPointer m_CurrentSelection;
};




#endif
