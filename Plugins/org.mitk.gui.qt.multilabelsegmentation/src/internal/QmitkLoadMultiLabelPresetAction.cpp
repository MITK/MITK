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
#include "QmitkLoadMultiLabelPresetAction.h"

#include "mitkLabelSetImage.h"
#include "mitkLabelSetIOHelper.h"
#include "mitkRenderingManager.h"

#include "QFileDialog"
#include "QInputDialog"
#include "QMessageBox"

#include "tinyxml.h"

QmitkLoadMultiLabelPresetAction::QmitkLoadMultiLabelPresetAction()
{
}

QmitkLoadMultiLabelPresetAction::~QmitkLoadMultiLabelPresetAction()
{
}

void QmitkLoadMultiLabelPresetAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  foreach ( mitk::DataNode::Pointer referenceNode, selectedNodes )
  {

    if (referenceNode.IsNull()) return;

    mitk::LabelSetImage::Pointer referenceImage = dynamic_cast<mitk::LabelSetImage*>( referenceNode->GetData() );
    assert(referenceImage);

    std::string sName = referenceNode->GetName();
    QString qName;
    qName.sprintf("%s.lsetp",sName.c_str());
    QString filename = QFileDialog::getOpenFileName(NULL,"Load file",QString(),"LabelSet Preset(*.lsetp)");
    if ( filename.isEmpty() )
      return;

    std::string fileName = filename.toStdString();
    mitk::LabelSetIOHelper::LoadLabelSetImagePreset(fileName, referenceImage);
  }
}

void QmitkLoadMultiLabelPresetAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkLoadMultiLabelPresetAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}

void QmitkLoadMultiLabelPresetAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkLoadMultiLabelPresetAction::SetDecimated(bool decimated)
{
  //not needed
}
