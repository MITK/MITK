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
#include "QmitkNewSegmentationDialog.h"
#include "mitkSegTool2D.h"
#include "mitkToolManagerProvider.h"

#include "mitkIOMimeTypes.h"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp> 
#include <boost/lexical_cast.hpp>

const std::string SegmentationReworkView::VIEW_ID = "org.mitk.views.segmentationreworkview";

void SegmentationReworkView::SetFocus() {}

void SegmentationReworkView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;

  qRegisterMetaType< std::vector<std::string> >("std::vector<std::string>");

  connect(m_Controls.buttonUpload, &QPushButton::clicked, this, &SegmentationReworkView::UploadNewSegmentation);
  connect(m_Controls.buttonNewSeg, &QPushButton::clicked, this, &SegmentationReworkView::CreateNewSegmentationC);
  connect(m_Controls.cleanDicomBtn, &QPushButton::clicked, this, &SegmentationReworkView::CleanDicomFolder);

  m_downloadBaseDir = std::experimental::filesystem::path("/temp/");
  m_tempSegDir = std::experimental::filesystem::path("/tempSeg/");

  if (!std::experimental::filesystem::exists(m_downloadBaseDir))
  {
    std::experimental::filesystem::create_directory(m_downloadBaseDir);
  }

  if (!std::experimental::filesystem::exists(m_tempSegDir))
  {
    std::experimental::filesystem::create_directory(m_tempSegDir);
  }

  utility::string_t port = U("2020");
  utility::string_t address = U("http://127.0.0.1:");
  address.append(port);

  m_HttpHandler = std::unique_ptr<SegmentationReworkREST>(new SegmentationReworkREST(address));

  connect(m_HttpHandler.get(),
    &SegmentationReworkREST::InvokeUpdateChartWidget,
    this,
    &SegmentationReworkView::UpdateChartWidget);
  connect(this, &SegmentationReworkView::InvokeLoadData, this, &SegmentationReworkView::LoadData);

  m_HttpHandler->SetPutCallback(std::bind(&SegmentationReworkView::RESTPutCallback, this, std::placeholders::_1));
  m_HttpHandler->SetGetCallback(std::bind(&SegmentationReworkView::RESTGetCallback, this, std::placeholders::_1));
  m_HttpHandler->Open().wait();

  MITK_INFO << "Listening for requests at: " << utility::conversions::to_utf8string(address);

  utility::string_t pacsHost = U("http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE");
  m_DICOMWeb = new mitk::DICOMWeb(pacsHost);
}

void SegmentationReworkView::RESTPutCallback(const SegmentationReworkREST::DicomDTO &dto)
{
  SetSimilarityGraph(dto.simScoreArray, dto.minSliceStart);

  typedef std::map<std::string, std::string> ParamMap;
  ParamMap seriesInstancesParams;

  seriesInstancesParams.insert((ParamMap::value_type({"StudyInstanceUID"}, dto.studyUID)));
  seriesInstancesParams.insert((ParamMap::value_type({"SeriesInstanceUID"}, dto.srSeriesUID)));
  seriesInstancesParams.insert((ParamMap::value_type({"includefield"}, {"0040A375"}))); // Current Requested Procedure Evidence Sequence

  try {
    m_DICOMWeb->QuidoRSInstances(seriesInstancesParams).then([=](web::json::value jsonResult) {

      auto firstResult = jsonResult[0];
      auto actualListKey = firstResult.at(U("0040A375")).as_object().at(U("Value")).as_array()[0].as_object().at(U("00081115")).as_object().at(U("Value")).as_array();

      std::string segSeriesUIDA = "";
      std::string segSeriesUIDB = "";
      std::string imageSeriesUID = "";

      for (unsigned int index = 0; index < actualListKey.size(); index++) {
        auto element = actualListKey[index].as_object();
        // get SOP class UID
        auto innerElement = element.at(U("00081199")).as_object().at(U("Value")).as_array()[0];
        auto sopClassUID = innerElement.at(U("00081150")).as_object().at(U("Value")).as_array()[0].as_string();

        auto seriesUID = utility::conversions::to_utf8string(element.at(U("0020000E")).as_object().at(U("Value")).as_array()[0].as_string());

        if (sopClassUID == L"1.2.840.10008.5.1.4.1.1.66.4") // SEG
        {
          if (segSeriesUIDA.length() == 0)
          {
            segSeriesUIDA = seriesUID;
          }
          else
          {
            segSeriesUIDB = seriesUID;
          }
        }
        else if (sopClassUID == L"1.2.840.10008.5.1.4.1.1.2")  // CT
        {
          imageSeriesUID = seriesUID;
        }
      }

      MITK_INFO << "image series UID " << imageSeriesUID;
      MITK_INFO << "seg A series UID " << segSeriesUIDA;
      MITK_INFO << "seg B series UID " << segSeriesUIDB;

      MITK_INFO << "Load related dicom series ...";
      boost::uuids::random_generator generator;

      std::string folderPathSeries = std::experimental::filesystem::path(m_downloadBaseDir).append(boost::uuids::to_string(generator())).string() + "/";
      std::experimental::filesystem::create_directory(folderPathSeries);

      std::string pathSegA = std::experimental::filesystem::path(m_downloadBaseDir).append(boost::uuids::to_string(generator())).string() + "/";
      std::string pathSegB = std::experimental::filesystem::path(m_downloadBaseDir).append(boost::uuids::to_string(generator())).string() + "/";

      auto folderPathSegA = utility::conversions::to_string_t(pathSegA);
      auto folderPathSegB = utility::conversions::to_string_t(pathSegB);

      std::experimental::filesystem::create_directory(pathSegA);
      std::experimental::filesystem::create_directory(pathSegB);

      m_CurrentStudyUID = dto.studyUID;

      std::vector<pplx::task<std::string>> tasks;
      auto imageSeriesTask = m_DICOMWeb->WadoRS(utility::conversions::to_string_t(folderPathSeries), dto.studyUID, imageSeriesUID);
      auto segATask = m_DICOMWeb->WadoRS(folderPathSegA, dto.studyUID, segSeriesUIDA);
      auto segBTask = m_DICOMWeb->WadoRS(folderPathSegB, dto.studyUID, segSeriesUIDB);
      tasks.push_back(imageSeriesTask);
      tasks.push_back(segATask);
      tasks.push_back(segBTask);

      auto joinTask = pplx::when_all(begin(tasks), end(tasks));
      auto filePathList = joinTask.then([&](std::vector<std::string> filePathList) {
        auto fileNameA = std::experimental::filesystem::path(filePathList[1]).filename();
        auto fileNameB = std::experimental::filesystem::path(filePathList[2]).filename();
        m_Controls.labelSegA->setText(m_Controls.labelSegA->text() + " " + QString::fromUtf8(fileNameA.string().c_str()));
        m_Controls.labelSegB->setText(m_Controls.labelSegB->text() + " " + QString::fromUtf8(fileNameB.string().c_str()));
        InvokeLoadData(filePathList);
      });

    });

  }
  catch (mitk::Exception&  e) {
    MITK_ERROR << e.what();
  }
}

void SegmentationReworkView::RESTGetCallback(const SegmentationReworkREST::DicomDTO &dto) 
{
  boost::uuids::random_generator generator;

  std::string folderPathSeries = std::experimental::filesystem::path(m_downloadBaseDir).append(boost::uuids::to_string(generator())).string() + "/";
  std::experimental::filesystem::create_directory(folderPathSeries);

  std::string pathSeg = std::experimental::filesystem::path(m_downloadBaseDir).append(boost::uuids::to_string(generator())).string() + "/";
  auto folderPathSeg = utility::conversions::to_string_t(pathSeg);
  std::experimental::filesystem::create_directory(pathSeg);

  std::vector<pplx::task<std::string>> tasks;
  auto imageSeriesTask = m_DICOMWeb->WadoRS(utility::conversions::to_string_t(folderPathSeries), dto.studyUID, dto.imageSeriesUID);
  auto segATask = m_DICOMWeb->WadoRS(folderPathSeg, dto.studyUID, dto.segSeriesUIDA);
  tasks.push_back(imageSeriesTask);
  tasks.push_back(segATask);

  auto joinTask = pplx::when_all(begin(tasks), end(tasks));
  auto filePathList = joinTask.then([&](std::vector<std::string> filePathList) { InvokeLoadData(filePathList); });
}

void SegmentationReworkView::LoadData(std::vector<std::string> filePathList)
{
  MITK_INFO << "Loading finished. Pushing data to data storage ...";
  auto ds = GetDataStorage();
  auto dataNodes = mitk::IOUtil::Load(filePathList, *ds);
  // reinit view
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(ds);

  // find data nodes

  m_Image = dataNodes->at(0);
  m_SegA = dataNodes->at(1);
  m_SegB = dataNodes->at(2);

}

void SegmentationReworkView::UpdateChartWidget()
{
  m_Controls.chartWidget->Show();
}

void SegmentationReworkView::SetSimilarityGraph(std::vector<double> simScoreArray, int sliceMinStart)
{
  std::map<double, double> map;

  double sliceIndex = sliceMinStart;
  for (double score : simScoreArray)
  {
    map.insert(std::map<double, double>::value_type(sliceIndex, score));
    sliceIndex++;
  }
  m_Controls.chartWidget->AddData2D(map, "similarity score");
  m_Controls.chartWidget->SetChartType("similarity score", QmitkChartWidget::ChartType::line);
  m_Controls.chartWidget->SetXAxisLabel("slice number");
  m_Controls.chartWidget->SetYAxisLabel("similarity score");
}

void SegmentationReworkView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach(mitk::DataNode::Pointer node, nodes)
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
  boost::uuids::random_generator generator;

  std::string folderPathSeg = std::experimental::filesystem::path(m_tempSegDir).append(boost::uuids::to_string(generator())).string() + "/";
  std::experimental::filesystem::create_directory(folderPathSeg);

  const std::string savePath = folderPathSeg + m_SegC->GetName() + ".dcm";
  const std::string mimeType = mitk::IOMimeTypes::DICOM_MIMETYPE_NAME();
  mitk::IOUtil::Save(m_SegC->GetData(), mimeType, savePath);

  auto filePath = utility::conversions::to_string_t(savePath);
  try {
    m_DICOMWeb->StowRS(filePath, m_CurrentStudyUID).wait();
  }
  catch (const std::exception &exception)
  {
    std::cout << exception.what() << std::endl;
  }
}

void SegmentationReworkView::CreateNewSegmentationC() 
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->InitializeTools();
  toolManager->SetReferenceData(m_Image);
  
  mitk::Image::Pointer baseImage;
  if (m_Controls.radioA->isChecked()) {
    baseImage = dynamic_cast<mitk::Image*>(m_SegA->GetData());
  }
  else if (m_Controls.radioB->isChecked()) {
    baseImage = dynamic_cast<mitk::Image*>(m_SegB->GetData());
  }

  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(m_Parent); // needs a QWidget as parent, "this" is not QWidget

  int dialogReturnValue = dialog->exec();
  if (dialogReturnValue == QDialog::Rejected)
  {
    // user clicked cancel or pressed Esc or something similar
    return;
  }

  // ask the user about an organ type and name, add this information to the image's (!) propertylist
  // create a new image of the same dimensions and smallest possible pixel type
  mitk::Tool* firstTool = toolManager->GetToolById(0);
  if (firstTool)
  {
    try
    {
      std::string newNodeName = dialog->GetSegmentationName().toStdString();
      if (newNodeName.empty())
      {
        newNodeName = "no_name";
      }

      mitk::DataNode::Pointer newSegmentation = firstTool->CreateSegmentationNode(baseImage, newNodeName, dialog->GetColor());
      // initialize showVolume to false to prevent recalculating the volume while working on the segmentation
      newSegmentation->SetProperty("showVolume", mitk::BoolProperty::New(false));
      if (!newSegmentation)
      {
        return; // could be aborted by user
      }

      if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0))
      {
        mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0)->SetSelected(false);
      }
      newSegmentation->SetSelected(true);
      this->GetDataStorage()->Add(newSegmentation, toolManager->GetReferenceData(0)); // add as a child, because the segmentation "derives" from the original
     
      m_SegC = newSegmentation;
    }
    catch (std::bad_alloc)
    {
      QMessageBox::warning(nullptr, tr("Create new segmentation"), tr("Could not allocate memory for new segmentation"));
    }
  }
  else {
    MITK_INFO << "no tools...";
  }
}

void SegmentationReworkView::CleanDicomFolder() 
{
  std::experimental::filesystem::remove_all(m_downloadBaseDir);
  std::experimental::filesystem::create_directory(m_downloadBaseDir);
}
