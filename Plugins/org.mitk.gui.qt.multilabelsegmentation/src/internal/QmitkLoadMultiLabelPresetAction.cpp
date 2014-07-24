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
#include "mitkRenderingManager.h"

#include "QInputDialog"
#include "QMessageBox"
#include "QFileDialog"
#include "mitkLabelSetImageReader.h"
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
    if (referenceNode.IsNotNull())
    {

      mitk::LabelSetImage* referenceImage = dynamic_cast<mitk::LabelSetImage*>( referenceNode->GetData() );
      assert(referenceImage);

      std::string sName = referenceNode->GetName();
      QString qName;
      qName.sprintf("%s.lsetp",sName.c_str());
      QString filename = QFileDialog::getOpenFileName(NULL,"Load file",QString(),"LabelSet Preset(*.lsetp)");
      if ( filename.isEmpty() )
        return;

      std::auto_ptr<TiXmlDocument> presetXmlDoc;
      presetXmlDoc.reset( new TiXmlDocument());

      bool ok = presetXmlDoc->LoadFile(filename.toStdString());
      if ( !ok )
        return;

      TiXmlElement * presetElem = presetXmlDoc->FirstChildElement("LabelSetImagePreset");
      if(!presetElem)
      {
        MITK_INFO << "No valid preset XML";
        return;
      }


      int numberOfLayers;
      presetElem->QueryIntAttribute("layers", &numberOfLayers);

      for(int i = 0 ; i < numberOfLayers; i++)
      {
        TiXmlElement * layerElem = presetElem->FirstChildElement("Layer");
        int numberOfLabels;
        layerElem->QueryIntAttribute("labels", &numberOfLabels);

        if(referenceImage->GetLabelSet(i) == NULL) referenceImage->AddLayer();

        TiXmlElement * labelElement = layerElem->FirstChildElement("Label");

        for(int j = 0 ; j < numberOfLabels; j++)
        {
          TiXmlPrinter p;
          labelElement->Accept(&p);
          MITK_INFO << p.CStr();

          mitk::Label::Pointer label = mitk::LabelSetImageReader::LoadLabelFromTiXmlDocument(labelElement);
          referenceImage->GetLabelSet()->AddLabel(label);

          labelElement = labelElement->NextSiblingElement("Label");
        }
      }
    }
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
