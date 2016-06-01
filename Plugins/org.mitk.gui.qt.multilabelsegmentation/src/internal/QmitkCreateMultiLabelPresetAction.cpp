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

#include <qcoreapplication.h>
#include "QFileDialog"
#include "QMessageBox"

#include "tinyxml.h"

#include "mitkLabelSetImage.h"
#include "mitkLabelSetIOHelper.h"
#include "QmitkCreateMultiLabelPresetAction.h"

QmitkCreateMultiLabelPresetAction::QmitkCreateMultiLabelPresetAction()
{
}

QmitkCreateMultiLabelPresetAction::~QmitkCreateMultiLabelPresetAction()
{
}

void QmitkCreateMultiLabelPresetAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  foreach ( mitk::DataNode::Pointer referenceNode, selectedNodes )
  {
    if (referenceNode.IsNotNull())
    {
      mitk::LabelSetImage::Pointer referenceImage = dynamic_cast<mitk::LabelSetImage*>( referenceNode->GetData() );
      assert(referenceImage);

      if(referenceImage->GetNumberOfLabels() <= 1)
      {
        QMessageBox::information(NULL, "Create LabelSetImage Preset", "Could not create a LabelSetImage preset.\nNo Labels defined!\n");\
        return;
      }

      std::string sName = referenceNode->GetName();
      QString qName;
      qName.sprintf("%s.lsetp",sName.c_str());
      QString filename = QFileDialog::getSaveFileName( NULL,"save file dialog",QString(),"LabelSet Preset(*.lsetp)");
      if ( filename.isEmpty() )
        return;

      std::string fileName = filename.toStdString();
      bool wasSaved = mitk::LabelSetIOHelper::SaveLabelSetImagePreset(fileName,referenceImage);

      if(!wasSaved)
      {
        QMessageBox::information(NULL, "Create LabelSetImage Preset", "Could not save a LabelSetImage preset as Xml.\n");\
        return;
      }
    }
  }
}

void QmitkCreateMultiLabelPresetAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreateMultiLabelPresetAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}

void QmitkCreateMultiLabelPresetAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkCreateMultiLabelPresetAction::SetDecimated(bool decimated)
{
  //not needed
}
