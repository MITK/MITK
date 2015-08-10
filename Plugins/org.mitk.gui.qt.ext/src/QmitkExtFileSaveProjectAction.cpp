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
#include <berryIPreferences.h>
#include "berryPlatform.h"

// DUPLICATED FROM QmitkFileOpenAction & QmitkExtFileOpenProjectAction
static berry::IPreferences::Pointer GetPreferences()
{
    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    if (prefService)
    {
        return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(0);
}

static QString getLastFileOpenPath()
{
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
        return prefs->Get("LastProjectFileOpenPath", "");
    }
    return QString();
}

static void setLastFileOpenPath(const QString& path)
{
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
        prefs->Put("LastProjectFileOpenPath", path);
        prefs->Flush();
    }
}
// end DUPLICATED


QmitkExtFileSaveProjectAction::QmitkExtFileSaveProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window, mitk::SceneIO::Pointer sceneIO, bool saveAs)
  : QAction(0)
  , m_Window(nullptr)
  , m_SceneIO(sceneIO)
  , m_SaveAs(saveAs)
{
  this->Init(window.GetPointer());
}

QmitkExtFileSaveProjectAction::QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO, bool saveAs)
  : QAction(0)
  , m_Window(nullptr)
  , m_SceneIO(sceneIO)
  , m_SaveAs(saveAs)
{
  this->Init(window);
}

void QmitkExtFileSaveProjectAction::Init(berry::IWorkbenchWindow* window)
{
  m_Window = window;
  this->setText(m_SaveAs ? "Save Project &As..." : "Save Project");
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
    mitk::IDataStorageReference::Pointer dsRef;

    {
      ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
      mitk::IDataStorageService* dss = 0;
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

    QString fileName;
    if (m_SaveAs || m_SceneIO->GetLoadedProjectFileName().empty()) {
        QString dialogTitle = "Save MITK Scene (%1)";
        fileName = QFileDialog::getSaveFileName(NULL,
            dialogTitle.arg(dsRef->GetLabel()),
            getLastFileOpenPath(),
            "MITK scene files (*.mitk)",
            NULL);
    }
    else {
        fileName = QString::fromStdString(m_SceneIO->GetLoadedProjectFileName());
    }

    if (fileName.isEmpty())
        return;

    if ( fileName.right(5) != ".mitk" )
      fileName += ".mitk";

    setLastFileOpenPath(fileName);

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

    /* Build list of nodes that should be saved */
    mitk::NodePredicateNot::Pointer isNotHelperObject =
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = storage->GetSubset(isNotHelperObject);

    if ( !m_SceneIO->SaveScene( nodesToBeSaved, storage, fileName.toStdString() ) )
    {
      QMessageBox::information(NULL,
                               "Scene saving",
                               "Scene could not be written completely. Please check the log.",
                               QMessageBox::Ok);

    }
    mitk::ProgressBar::GetInstance()->Progress(2);

    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = m_SceneIO->GetFailedNodes();
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
          ss << "(NULL)";
        }

        ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }

    mitk::PropertyList::ConstPointer failedProperties = m_SceneIO->GetFailedProperties();
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
    emit projectSaved(fileName);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene saving: " << e.what();
  }
}
