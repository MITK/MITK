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
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"
#include "mitkProgressBar.h"

#include <QFileDialog>
#include <QMessageBox>

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
    connect( (QObject*)(m_Controls->btnLoadSceneFile),     SIGNAL(clicked()), this, SLOT(LoadSceneFile()) );
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
  QString sceneFilename;
  try
  {
    // first ask user for a filename 
    bool niceFilenameFound(false);
    while (!niceFilenameFound)
    {
      sceneFilename = QFileDialog::getSaveFileName( m_Parent, "Save MITK scene", QString::null, "MITK scene files (*.mitk)", NULL );

      if (sceneFilename.isEmpty()) 
        return;

      if ( sceneFilename.right(5) != ".mitk" ) 
        sceneFilename += ".mitk";

      std::ifstream exists( sceneFilename.toLocal8Bit().constData() );
      if (exists)
      {
        int result = QMessageBox::question(NULL,
                                           "Overwrite file?", 
                                           sceneFilename + " already exists. \n\nDo you want to OVERWRITE this file?",
                                           QMessageBox::Yes,
                                           QMessageBox::No,
                                           QMessageBox::Cancel);

        if (result == QMessageBox::Cancel) 
          return;

        niceFilenameFound = QMessageBox::Yes == result;
      }
      else
      {
        niceFilenameFound = true;
      }
    }

    // really save now
    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    /*  TODO respect user selection in list view
     *  intermediate step: create new data storage, add all selected nodes
     *  OR: create a predicate ORing all selected nodes
     */
    mitk::DataStorage::Pointer storage = this->GetDefaultDataStorage();

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
    if ( !sceneIO->SaveScene( storage->GetAll(), storage, sceneFilename.toLocal8Bit().constData() ) )
    {
      QMessageBox::information(NULL,
                            "Scene saving",
                            "Scene could not be written completely. Please check the log.",
                            QMessageBox::Ok);

    }
    mitk::ProgressBar::GetInstance()->Progress(2);

    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
    if (failedNodes.IsNotNull())
    {
      std::stringstream ss;
      ss << "The following nodes could not be serialized:" << std::endl;
      for ( mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
            iter != failedNodes->end();
            ++iter )
      {
        ss << " - ";
        if ( mitk::BaseData* data =(*iter)->GetData() )
        {
          ss << data->GetNameOfClass();
        }
        else
        {
          ss << "(NULL)";
        }

        ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
      }

      LOG_WARN << ss.str();
    }

    mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
    if (failedProperties.IsNotNull())
    {
      std::stringstream ss;
      ss << "The following properties could not be serialized:" << std::endl;
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
            iter != propmap->end();
            ++iter )
      {
        ss << " - " << iter->second.first->GetNameOfClass() << " associated to key '" << iter->first << "'" << std::endl;
      }

      LOG_WARN << ss.str();
    }
  }
  catch (std::exception& e)
  {
    LOG_ERROR << "Exception caught during scene saving: " << e.what();
  }

}

void QmitkSceneSerializationView::LoadSceneFile()
{
  /* ask the user for a file name */
  QString sceneFilename = QFileDialog::getOpenFileName( m_Parent, "Load MITK scene", QDir::currentPath(), "MITK scene files (*.mitk)", NULL );

  if (sceneFilename.isEmpty()) return;

  mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

  mitk::DataStorage::Pointer storage = this->GetDefaultDataStorage();

  bool clearDataStorageFirst(false);
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
  mitk::DataStorage::Pointer newStorage = sceneIO->LoadScene( sceneFilename.toLocal8Bit().constData(), storage, clearDataStorageFirst );
  mitk::ProgressBar::GetInstance()->Progress(2);

  // perform reset on multiwidget
  /* get all nodes that have not set "includeInBoundingBox" to false */
  mitk::NodePredicateNOT::Pointer pred = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = storage->GetSubset(pred);
  /* calculate bounding geometry of these nodes */
  mitk::TimeSlicedGeometry::Pointer bounds = storage->ComputeBoundingGeometry3D(rs);
  /* initialize the views to the bounding geometry */
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

  assert( newStorage.GetPointer() == NULL || newStorage.GetPointer() == storage.GetPointer() );
}
