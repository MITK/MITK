/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    QString filename = QFileDialog::getOpenFileName(nullptr,"Load file",QString(),"LabelSet Preset(*.lsetp)");
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

void QmitkLoadMultiLabelPresetAction::SetSmoothed(bool)
{
  //not needed
}

void QmitkLoadMultiLabelPresetAction::SetDecimated(bool)
{
  //not needed
}
