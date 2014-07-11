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
#include "QmitkCreateMultiLabelSegmentationAction.h"

#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"

#include "QInputDialog"
#include "QMessageBox"

#include "QmitkNewSegmentationDialog.h"
#include "QmitkMultiLabelSegmentationView.h"
#include "QmitkMultiLabelSegmentationOrganNamesHandling.cpp"
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
  foreach ( mitk::DataNode::Pointer referenceNode, selectedNodes )
  {
    if (referenceNode.IsNotNull())
    {

      mitk::Image* referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
      assert(referenceImage);

      QString newName = QString::fromStdString(referenceNode->GetName());
      newName.append("-labels");

      bool ok = false;
      newName = QInputDialog::getText(NULL, "New Segmentation Session", "New name:", QLineEdit::Normal, newName, &ok);
      if(!ok) return;

      mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();

      try
      {
        workingImage->Initialize(referenceImage);
      }
      catch ( mitk::Exception& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        QMessageBox::information(NULL, "New Segmentation Session", "Could not create a new segmentation session.\n");
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

void QmitkCreateMultiLabelSegmentationAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetDecimated(bool decimated)
{
  //not needed
}
