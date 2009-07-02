/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkSceneSerializationView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkDataStorageTableModel.h"

#include "mitkSceneIO.h"


const std::string QmitkSceneSerializationView::VIEW_ID = "org.mitk.views.sceneserialization";

QmitkSceneSerializationView::QmitkSceneSerializationView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
}

QmitkSceneSerializationView::~QmitkSceneSerializationView()
{
}

void QmitkSceneSerializationView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkSceneSerializationViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // define data type for combobox
    QmitkDataStorageTableModel* model = new QmitkDataStorageTableModel( this->GetDefaultDataStorage() );
    m_Controls->lstScene->setModel( model );
  }
}

void QmitkSceneSerializationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkSceneSerializationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkSceneSerializationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->btnSerializeSelected), SIGNAL(clicked()), this, SLOT(SerializeSelected()) );
  }
}

void QmitkSceneSerializationView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkSceneSerializationView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkSceneSerializationView::SerializeSelected()
{
  mitk::SceneIO::Pointer sceneio = mitk::SceneIO::New();

  /* 
   *  intermediate step: create new data storage, add all selected nodes
   *  OR: create a predicate ORing all selected nodes
   */
  mitk::DataStorage::Pointer storage = this->GetDefaultDataStorage();

  sceneio->SaveScene( storage, "scene.zip" );
}


