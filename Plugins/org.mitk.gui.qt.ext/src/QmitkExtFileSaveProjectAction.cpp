/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtFileSaveProjectAction.h"

#include "internal/QmitkCommonExtPlugin.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include <mitkSceneIO.h>
#include <mitkProgressBar.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include "berryPlatform.h"


QmitkExtFileSaveProjectAction::QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->Init(window.GetPointer());
}

QmitkExtFileSaveProjectAction::QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->Init(window);
}

void QmitkExtFileSaveProjectAction::Init(berry::IWorkbenchWindow* window)
{
  m_Window = window;
  this->setText("&Save Project...");
  this->setToolTip("Save content of Data Manager as a .mitk project file");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}


void QmitkExtFileSaveProjectAction::Run()
{
  try
  {
    /**
     * @brief stores the last path of last saved file
     */
    static QString m_LastPath;

    mitk::IDataStorageReference::Pointer dsRef;

    {
      ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
      mitk::IDataStorageService* dss = nullptr;
      ctkServiceReference dsServiceRef = context->getServiceReference<mitk::IDataStorageService>();
      if (dsServiceRef)
      {
        dss = context->getService<mitk::IDataStorageService>(dsServiceRef);
      }

      if (!dss)
      {
        QString msg = "IDataStorageService service not available. Unable to open files.";
        MITK_WARN << msg.toStdString();
        QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
        return;
      }

      // Get the active data storage (or the default one, if none is active)
      dsRef = dss->GetDataStorage();
      context->ungetService(dsServiceRef);
    }

    mitk::DataStorage::Pointer storage = dsRef->GetDataStorage();

    QString dialogTitle = "Save MITK Scene (%1)";
    QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                    dialogTitle.arg(dsRef->GetLabel()),
                                                    m_LastPath,
                                                    "MITK scene files (*.mitk)",
                                                    nullptr );

    if (fileName.isEmpty() )
      return;

    // remember the location
    m_LastPath = fileName;

    if ( fileName.right(5) != ".mitk" )
      fileName += ".mitk";

    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

    /* Build list of nodes that should be saved */
    mitk::NodePredicateNot::Pointer isNotHelperObject =
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = storage->GetSubset(isNotHelperObject);

    if ( !sceneIO->SaveScene( nodesToBeSaved, storage, fileName.toStdString() ) )
    {
      QMessageBox::information(nullptr,
                               "Scene saving",
                               "Scene could not be written completely. Please check the log.",
                               QMessageBox::Ok);

    }
    mitk::ProgressBar::GetInstance()->Progress(2);

    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
    if (!failedNodes->empty())
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
          ss << "(nullptr)";
        }

        ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }

    mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
    if (!failedProperties->GetMap()->empty())
    {
      std::stringstream ss;
      ss << "The following properties could not be serialized:" << std::endl;
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
      {
        ss << " - " << iter->second->GetNameOfClass() << " associated to key '" << iter->first << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene saving: " << e.what();
  }
}
