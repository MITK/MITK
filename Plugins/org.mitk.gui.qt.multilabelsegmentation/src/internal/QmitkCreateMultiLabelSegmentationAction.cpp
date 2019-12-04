/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkCreateMultiLabelSegmentationAction.h"

#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"

#include "QInputDialog"
#include "QMessageBox"

#include "QmitkNewSegmentationDialog.h"
#include "QmitkMultiLabelSegmentationView.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"
//needed for qApp
#include <qcoreapplication.h>

QmitkCreateMultiLabelSegmentationAction::QmitkCreateMultiLabelSegmentationAction()
{
}

QmitkCreateMultiLabelSegmentationAction::~QmitkCreateMultiLabelSegmentationAction()
{
}

void QmitkCreateMultiLabelSegmentationAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  if (m_DataStorage.IsNull())
  {
    auto message = tr("Data storage not set.");
    MITK_ERROR << message;
    QMessageBox::warning(nullptr, "New Segmentation Session", message);
    return;
  }

  for ( auto referenceNode : selectedNodes )
  {
    if (referenceNode.IsNotNull())
    {
      mitk::Image* referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
      if (nullptr == referenceImage)
      {
        MITK_WARN << "Could not create multi label segmentation for non-image node - skipping action.";
        continue;
      }

      QString newName = QString::fromStdString(referenceNode->GetName());
      newName.append("-labels");

      bool ok = false;
      newName = QInputDialog::getText(nullptr, "New Segmentation Session", "New name:", QLineEdit::Normal, newName, &ok);
      if(!ok) return;

      mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();

      try
      {
        workingImage->Initialize(referenceImage);
      }
      catch ( mitk::Exception& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        QMessageBox::warning(nullptr, "New Segmentation Session", "Could not create a new segmentation session.");
        return;
      }

      mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
      workingNode->SetData(workingImage);
      workingNode->SetName(newName.toStdString());

      // set additional image information
      workingImage->GetExteriorLabel()->SetProperty("name.parent",mitk::StringProperty::New(referenceNode->GetName().c_str()));
      workingImage->GetExteriorLabel()->SetProperty("name.image",mitk::StringProperty::New(newName.toStdString().c_str()));

      if (!m_DataStorage->Exists(workingNode))
        m_DataStorage->Add(workingNode, referenceNode);

      QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( );
      dialog->SetSuggestionList( mitk::OrganNamesHandling::GetDefaultOrganColorString());
      dialog->setWindowTitle("New Label");

      int dialogReturnValue = dialog->exec();

      if ( dialogReturnValue == QDialog::Rejected ) return;

      QString segName = dialog->GetSegmentationName();
      if(segName.isEmpty()) segName = "Unnamed";
      workingImage->GetActiveLabelSet()->AddLabel(segName.toStdString(), dialog->GetColor());

    }
  }
}

void QmitkCreateMultiLabelSegmentationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreateMultiLabelSegmentationAction::SetFunctionality(berry::QtViewPart*)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetSmoothed(bool)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetDecimated(bool)
{
  //not needed
}
