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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "SegmentationReworkView.h"

// Qt
#include <QMessageBox>

#include <mitkIOUtil.h>

const std::string SegmentationReworkView::VIEW_ID = "org.mitk.views.segmentationreworkview";

void SegmentationReworkView::SetFocus()
{

}

void SegmentationReworkView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  connect(m_Controls.buttonUpload, &QPushButton::clicked, this, &SegmentationReworkView::UploadNewSegmentation);

  utility::string_t port = U("2020");
  utility::string_t address = U("http://127.0.0.1:");
  address.append(port);

  m_HttpHandler = std::unique_ptr<SegmentationReworkREST>(new SegmentationReworkREST(address));

  m_HttpHandler->SetPutCallback(std::bind(&SegmentationReworkView::RESTPutCallback, this, std::placeholders::_1));
  m_HttpHandler->Open().wait();

  MITK_INFO << "Listening for requests at: " << utility::conversions::to_utf8string(address);

  utility::string_t pacsHost = U("http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE");
  m_RestClient = new mitk::RESTClient(pacsHost);
}

void SegmentationReworkView::RESTPutCallback(SegmentationReworkREST::DicomDTO& dto)
{
  MITK_INFO << "callback! " << dto.studyUID << dto.seriesUID << dto.instanceUID;
  SetSimilarityGraph(dto.simScoreArray, dto.minSliceStart);

  auto filePath = U("/temp/downloadWADO.dcm");

  m_RestClient->executeWADOGET(filePath, dto.studyUID, dto.seriesUID, dto.instanceUID);

  MITK_INFO << "load file into data storage";
  mitk::IOUtil::Load(utility::conversions::to_utf8string(filePath), *this->GetDataStorage());
  this->GetDataStorage()->GetAll()->at(0)->Update();
}

void SegmentationReworkView::SetSimilarityGraph(std::vector<double> simScoreArray, int sliceMinStart)
{
  std::map<double, double> map;
  MITK_INFO << simScoreArray.size();

  double sliceIndex = sliceMinStart;
  for (double score : simScoreArray) {
    map.insert(std::map<double, double>::value_type(sliceIndex, score));
    sliceIndex++;
  }
  m_Controls.chartWidget->AddData2D(map, "similarity score");
  m_Controls.chartWidget->SetChartType("similarity score", QmitkChartWidget::ChartType::line);
  m_Controls.chartWidget->SetXAxisLabel("slice number");
  m_Controls.chartWidget->SetYAxisLabel("similarity score");
  MITK_INFO << "before show";
  //m_Controls.chartWidget->Show();
  MITK_INFO << "after show";
}


void SegmentationReworkView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
    {
      m_Controls.labelWarning->setVisible(false);
      return;
    }
  }

  m_Controls.labelWarning->setVisible(true);
}

void SegmentationReworkView::UploadNewSegmentation()
{

}

void SegmentationReworkView::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;

  mitk::DataNode *node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(nullptr, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // actually do something here...
    }
  }
}
