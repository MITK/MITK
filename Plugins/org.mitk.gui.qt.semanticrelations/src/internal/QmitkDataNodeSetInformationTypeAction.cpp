/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkDataNodeSetInformationTypeAction.h"
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"

// semantic relations module
#include <mitkControlPointManager.h>
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsDataStorageAccess.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkSemanticRelationsIntegration.h>

// qt
#include <QInputDialog>
#include <QMessageBox>

QmitkDataNodeSetInformationTypeAction::QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Set information type"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetInformationTypeAction::QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Set information type"));
  m_Parent = parent;
  InitializeAction();
}

void QmitkDataNodeSetInformationTypeAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeSetInformationTypeAction::OnActionTriggered);
}

void QmitkDataNodeSetInformationTypeAction::OnActionTriggered(bool /*checked*/)
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  QInputDialog* inputDialog = new QInputDialog(m_Parent);
  inputDialog->setWindowTitle(tr("Set information type of selected node"));
  inputDialog->setLabelText(tr("Information type:"));
  inputDialog->setTextValue(QString::fromStdString(mitk::SemanticRelationsInference::GetInformationTypeOfImage(dataNode)));
  inputDialog->setMinimumSize(250, 100);

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::SemanticTypes::InformationType informationType = inputDialog->textValue().toStdString();

  mitk::SemanticTypes::ExaminationPeriod examinationPeriod;
  mitk::SemanticTypes::ControlPoint controlPoint;
  mitk::SemanticRelationsDataStorageAccess::DataNodeVector allSpecificImages;
  try
  {
    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(dataNode);
    controlPoint = mitk::SemanticRelationsInference::GetControlPointOfImage(dataNode);
    // see if the examination period - information type cell is already taken
    examinationPeriod = mitk::FindFittingExaminationPeriod(caseID, controlPoint);
    auto semanticRelationsDataStorageAccess = mitk::SemanticRelationsDataStorageAccess(dataStorage);
    try
    {
      allSpecificImages = semanticRelationsDataStorageAccess.GetAllSpecificImages(caseID, informationType, examinationPeriod);
    }
    catch (const mitk::SemanticRelationException&)
    {
      // just continue since an exception means that there is no specific image
    }
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox(QMessageBox::Warning,
      "Could not set the information type.",
      "The program wasn't able to correctly set the information type.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str() + "\n"));
    msgBox.exec();
  }

  if (!allSpecificImages.empty())
  {
    // examination period - information type cell is already taken
    // ask if cell should be overwritten
    QMessageBox::StandardButton answerButton =
      QMessageBox::question(nullptr,
        "Specific image already exists.",
        QString::fromStdString("Force overwriting existing image " + informationType + " at " + examinationPeriod.name + "?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answerButton == QMessageBox::Yes)
    {
      try
      {
        // remove already existent images at specific cell
        for (const auto& specificImage : allSpecificImages)
        {
          RemoveFromSemanticRelationsAction::Run(dataStorage, specificImage);
        }
      }
      catch (const mitk::SemanticRelationException& e)
      {
        std::stringstream exceptionMessage; exceptionMessage << e;
        QMessageBox msgBox(QMessageBox::Warning,
          "Could not set the information type.",
          "The program wasn't able to correctly set the information type.\n"
          "Reason:\n" + QString::fromStdString(exceptionMessage.str() + "\n"));
        msgBox.exec();
      }
    }
    else
    {
      // else case is: no overwriting
      return;
    }
  }

  mitk::SemanticRelationsIntegration semanticRelationsIntegration;
  try
  {
    semanticRelationsIntegration.SetInformationType(dataNode, informationType);
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox(QMessageBox::Warning,
      "Could not set the information type.",
      "The program wasn't able to correctly set the information type.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str() + "\n"));
    msgBox.exec();
  }
}
