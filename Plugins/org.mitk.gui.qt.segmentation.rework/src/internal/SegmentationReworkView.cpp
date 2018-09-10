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

// mitk image
#include <mitkImage.h>

const std::string SegmentationReworkView::VIEW_ID = "org.mitk.views.segmentationreworkview";

void SegmentationReworkView::SetFocus()
{

}

void SegmentationReworkView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  connect(m_Controls.buttonUpload, &QPushButton::clicked, this, &SegmentationReworkView::UploadNewSegmentation);

  std::map<double, double> map;
  map.insert(std::map<double, double>::value_type(0,0.92));
  map.insert(std::map<double, double>::value_type(1, 0.32));
  map.insert(std::map<double, double>::value_type(2, 0.52));

  m_Controls.chartWidget->AddData2D(map, "similarity score");
  m_Controls.chartWidget->SetChartType("similarity score", QmitkChartWidget::ChartType::line);
  m_Controls.chartWidget->SetXAxisLabel("slice number");
  m_Controls.chartWidget->SetYAxisLabel("similarity score");
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  m_Controls.chartWidget->Show();
#endif


  utility::string_t port = U("2020");
  utility::string_t address = U("http://127.0.0.1:");
  address.append(port);

  m_HttpHandler = std::unique_ptr<SegmentationReworkREST>(new SegmentationReworkREST(address));
  m_HttpHandler->open().wait();

  MITK_INFO << "Listening for requests at: " << utility::conversions::to_utf8string(address);

  utility::string_t pacsHost = U("http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE");
  m_RestClient = new mitk::RESTClient(pacsHost);


  auto studyUID = "1.2.840.113654.2.70.1.212389164178151513355109725184839174882";
  auto seriesUID = "1.2.276.0.7230010.3.1.3.296485376.9.1536581245.922469";
  auto instanceUID = "1.2.276.0.7230010.3.1.4.296485376.9.1536581245.922470";

  m_RestClient->executeWADOGET(studyUID, seriesUID, instanceUID);
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
