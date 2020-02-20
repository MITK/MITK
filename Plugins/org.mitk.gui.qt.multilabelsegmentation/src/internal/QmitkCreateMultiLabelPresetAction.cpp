/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
        QMessageBox::information(nullptr, "Create LabelSetImage Preset", "Could not create a LabelSetImage preset.\nNo Labels defined!\n");\
        return;
      }

      std::string sName = referenceNode->GetName();
      QString qName;
      qName.sprintf("%s.lsetp",sName.c_str());
      QString filename = QFileDialog::getSaveFileName( nullptr,"save file dialog",QString(),"LabelSet Preset(*.lsetp)");
      if ( filename.isEmpty() )
        return;

      std::string fileName = filename.toStdString();
      bool wasSaved = mitk::LabelSetIOHelper::SaveLabelSetImagePreset(fileName,referenceImage);

      if(!wasSaved)
      {
        QMessageBox::information(nullptr, "Create LabelSetImage Preset", "Could not save a LabelSetImage preset as Xml.\n");\
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

void QmitkCreateMultiLabelPresetAction::SetSmoothed(bool)
{
  //not needed
}

void QmitkCreateMultiLabelPresetAction::SetDecimated(bool)
{
  //not needed
}
