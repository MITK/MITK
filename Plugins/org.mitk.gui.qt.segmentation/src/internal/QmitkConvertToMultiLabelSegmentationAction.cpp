/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkConvertToMultiLabelSegmentationAction.h"
#include "QmitkCreateMultiLabelSegmentationAction.h"

#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkRenderingManager.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>

//needed for qApp
#include <QCoreApplication>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  constexpr unsigned int DISTINCT_VALUE_WARNING_THRESHOLD = 100;
  constexpr auto WARN_PREF_KEY = "warn before converting to segmentation";
}

QmitkConvertToMultiLabelSegmentationAction::QmitkConvertToMultiLabelSegmentationAction()
{
}

QmitkConvertToMultiLabelSegmentationAction::~QmitkConvertToMultiLabelSegmentationAction()
{
}

void QmitkConvertToMultiLabelSegmentationAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  const bool warn = GetPreferences()->GetBool(WARN_PREF_KEY, true);

  for (const auto &referenceNode : selectedNodes)
  {
    if (referenceNode.IsNull())
      continue;

    auto* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
    if (nullptr == referenceImage)
    {
      MITK_WARN << "Could not convert to multi label segmentation for non-image node - skipping action.";
      continue;
    }

    if (warn &&
        mitk::CountDistinctForegroundValues(referenceImage, DISTINCT_VALUE_WARNING_THRESHOLD) >= DISTINCT_VALUE_WARNING_THRESHOLD)
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setWindowTitle(QStringLiteral("Convert to Segmentation"));
      msgBox.setText(QString("Image \"%1\" contains at least %2 distinct values.")
                       .arg(QString::fromStdString(referenceNode->GetName()))
                       .arg(DISTINCT_VALUE_WARNING_THRESHOLD));
      msgBox.setInformativeText(QStringLiteral(
        "Are you sure you meant to convert this image to a segmentation? "
        "Converting creates one label per distinct value, which can be slow and memory intensive."));

      auto* cancelButton = msgBox.addButton(QMessageBox::Cancel);
      auto* createButton = msgBox.addButton(QStringLiteral("Create new segmentation instead"), QMessageBox::AcceptRole);
      auto* convertButton = msgBox.addButton(QStringLiteral("Convert anyway"), QMessageBox::AcceptRole);

      auto* dontAskCheckBox = new QCheckBox(QStringLiteral("Do not ask again"));
      msgBox.setCheckBox(dontAskCheckBox); // QMessageBox takes ownership
      msgBox.setDefaultButton(cancelButton);

      msgBox.exec();
      auto* clickedButton = msgBox.clickedButton();

      // "Do not ask again" controls whether to keep warning, not which action
      // the user chose. Persist it whenever the user makes an actual choice
      // (Convert anyway or Create new segmentation), but not on Cancel or when
      // the dialog is dismissed.
      const bool userMadeChoice = clickedButton == convertButton || clickedButton == createButton;
      if (userMadeChoice && dontAskCheckBox->isChecked())
      {
        auto* prefs = GetPreferences();
        prefs->PutBool(WARN_PREF_KEY, false);
        prefs->Flush();
      }

      if (clickedButton == createButton)
      {
        QmitkCreateMultiLabelSegmentationAction createAction;
        createAction.SetDataStorage(m_DataStorage.GetPointer());
        createAction.Run(QList<mitk::DataNode::Pointer>{ referenceNode });
        continue;
      }

      if (clickedButton != convertButton)
        continue; // Cancel or dialog dismissed: do nothing
    }

    auto lsImage = mitk::MultiLabelSegmentation::New();
    try
    {
      lsImage->InitializeByLabeledImage(referenceImage);
    }
    catch (mitk::Exception &e)
    {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      continue;
    }

    if (m_DataStorage.IsNotNull())
    {
      auto newNode = mitk::DataNode::New();
      std::string newName = referenceNode->GetName();
      newName += "-labels";
      newNode->SetName(newName);
      newNode->SetData(lsImage);
      m_DataStorage->Add(newNode, referenceNode);
    }

    lsImage->Modified();
  }
}

void QmitkConvertToMultiLabelSegmentationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkConvertToMultiLabelSegmentationAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}
