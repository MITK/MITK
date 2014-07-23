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
#include "QmitkCreateMultiLabelPresetAction.h"
#include "mitkLabelSetImage.h"
#include "QMessageBox"
#include <qcoreapplication.h>
#include "QFileDialog"
#include "mitkLabelSetImageWriter.h"
#include "tinyxml.h"

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

      mitk::LabelSetImage* referenceImage = dynamic_cast<mitk::LabelSetImage*>( referenceNode->GetData() );
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

      std::auto_ptr<TiXmlDocument> presetXmlDoc;
      presetXmlDoc.reset( new TiXmlDocument());

      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
      presetXmlDoc->LinkEndChild( decl );

      TiXmlElement * presetElement = new TiXmlElement("LabelSetImagePreset");
      presetElement->SetAttribute("layers", referenceImage->GetNumberOfLayers());

      presetXmlDoc->LinkEndChild(presetElement);

      for (int layerIdx=0; layerIdx<referenceImage->GetNumberOfLayers(); layerIdx++)
      {
        TiXmlElement * layerElement = new TiXmlElement("Layer");
        layerElement->SetAttribute("index", layerIdx);
        layerElement->SetAttribute("labels", referenceImage->GetNumberOfLabels(layerIdx));

        presetElement->LinkEndChild(layerElement);

        for (int labelIdx=0; labelIdx<referenceImage->GetNumberOfLabels(layerIdx); labelIdx++)
        {
          TiXmlElement * labelAsXml = mitk::LabelSetImageWriter::GetLabelAsTiXmlElement(referenceImage->GetLabel(labelIdx,layerIdx));
          layerElement->LinkEndChild(labelAsXml);
        }
      }


      bool wasSaved = presetXmlDoc->SaveFile(filename.toStdString());
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
