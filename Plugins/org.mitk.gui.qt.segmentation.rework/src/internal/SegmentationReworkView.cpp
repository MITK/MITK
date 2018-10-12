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

#include <itkFileTools.h>

// Qt
#include <QMessageBox>

#include <mitkIOUtil.h>
#include "QmitkNewSegmentationDialog.h"
#include "mitkSegTool2D.h"
#include "mitkToolManagerProvider.h"

const std::string SegmentationReworkView::VIEW_ID = "org.mitk.views.segmentationreworkview";

void SegmentationReworkView::SetFocus() {}

void SegmentationReworkView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;
  counter = 0;

  qRegisterMetaType< std::vector<std::string> >("std::vector<std::string>");

  m_Controls.chartWidget->setVisible(false);
  m_Controls.verticalWidget->setVisible(false);
  m_Controls.cleanDicomBtn->setVisible(false);

  connect(m_Controls.buttonUpload, &QPushButton::clicked, this, &SegmentationReworkView::UploadNewSegmentation);
  connect(m_Controls.buttonNewSeg, &QPushButton::clicked, this, &SegmentationReworkView::CreateNewSegmentationC);
  connect(m_Controls.cleanDicomBtn, &QPushButton::clicked, this, &SegmentationReworkView::CleanDicomFolder);
  connect(m_Controls.restartConnection, &QPushButton::clicked, this, &SegmentationReworkView::RestartConnection);

  m_downloadBaseDir = mitk::IOUtil::GetTempPath();
  m_tempSegDir = "/tempSeg/";

  if (!itksys::SystemTools::FileExists(m_downloadBaseDir))
  {
    itk::FileTools::CreateDirectory(m_downloadBaseDir.c_str());
  }

  //if (!std::experimental::filesystem::exists(m_tempSegDir))
  //{
  //  std::experimental::filesystem::create_directory(m_tempSegDir);
  //}

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
  m_HttpHandler->SetGetImageSegCallback(std::bind(&SegmentationReworkView::RESTGetCallback, this, std::placeholders::_1));
  m_HttpHandler->Open().wait();

  MITK_INFO << "Listening for requests at: " << utility::conversions::to_utf8string(address);

  utility::string_t pacsHost = U("http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE");
  m_DICOMWeb = new mitk::DICOMWeb(pacsHost);
  MITK_INFO << "requests to pacs are sent to: " << utility::conversions::to_utf8string(pacsHost);
}

std::string SegmentationReworkView::getNextFolderName() {
  counter++;
  std::string name = std::string("foldername") + std::to_string(counter);
  MITK_INFO << name;
  return name;
}

void SegmentationReworkView::RestartConnection()
{
  auto host = m_Controls.dcm4cheeHostValue->text();
  std::string url = host.toStdString() + "/dcm4chee-arc/aets/DCM4CHEE";

  m_DICOMWeb = new mitk::DICOMWeb(utility::conversions::to_string_t(url));
  MITK_INFO << "requests to pacs are sent to: " << url;
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

        if (sopClassUID == U("1.2.840.10008.5.1.4.1.1.66.4")) // SEG
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
        else if (sopClassUID == U("1.2.840.10008.5.1.4.1.1.2"))  // CT
        {
          imageSeriesUID = seriesUID;
        }
      }

      MITK_INFO << "image series UID " << imageSeriesUID;
      MITK_INFO << "seg A series UID " << segSeriesUIDA;
      MITK_INFO << "seg B series UID " << segSeriesUIDB;

      MITK_INFO << "Load related dicom series ...";

      std::string folderPathSeries = m_downloadBaseDir + getNextFolderName() + "/";
      itk::FileTools::CreateDirectory(folderPathSeries);

      std::string pathSegA = m_downloadBaseDir + getNextFolderName() + "/";
      std::string pathSegB = m_downloadBaseDir + getNextFolderName() + "/";

      auto folderPathSegA = utility::conversions::to_string_t(pathSegA);
      auto folderPathSegB = utility::conversions::to_string_t(pathSegB);

      itk::FileTools::CreateDirectory(pathSegA);
      itk::FileTools::CreateDirectory(pathSegB);

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
        //auto fileNameA = std::experimental::filesystem::path(filePathList[1]).filename();
        //auto fileNameB = std::experimental::filesystem::path(filePathList[2]).filename();
        //m_Controls.labelSegA->setText(m_Controls.labelSegA->text() + " " + QString::fromUtf8(fileNameA.string().c_str()));
        //m_Controls.labelSegB->setText(m_Controls.labelSegB->text() + " " + QString::fromUtf8(fileNameB.string().c_str()));
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
  std::string folderPathSeries = m_downloadBaseDir + getNextFolderName() + "/";
  itk::FileTools::CreateDirectory(folderPathSeries);

  MITK_INFO << folderPathSeries;

  std::string pathSeg = m_downloadBaseDir + getNextFolderName() + "/";
  auto folderPathSeg = utility::conversions::to_string_t(pathSeg);
  itk::FileTools::CreateDirectory(pathSeg);

  MITK_INFO << pathSeg;

  std::vector<pplx::task<std::string>> tasks;
  auto imageSeriesTask = m_DICOMWeb->WadoRS(utility::conversions::to_string_t(folderPathSeries), dto.studyUID, dto.imageSeriesUID);
  
  tasks.push_back(imageSeriesTask);

  if (dto.segSeriesUIDList.size() > 0) {
    for (std::string segSeriesUID : dto.segSeriesUIDList)
    {
      auto segTask = m_DICOMWeb->WadoRS(folderPathSeg, dto.studyUID, segSeriesUID);
      tasks.push_back(segTask);
    }
  }
  else {
    auto segATask = m_DICOMWeb->WadoRS(folderPathSeg, dto.studyUID, dto.segSeriesUIDA);
    tasks.push_back(segATask);
  }


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
  if (dataNodes->size() > 2) {
    m_SegB = dataNodes->at(2);
  }
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
  m_Controls.chartWidget->AddData2D(map, "similarity graph");
  m_Controls.chartWidget->SetChartType("similarity graph", QmitkChartWidget::ChartType::line);
  m_Controls.chartWidget->SetXAxisLabel("slice number");
  m_Controls.chartWidget->SetYAxisLabel("similarity in percent");
}

void SegmentationReworkView::UploadNewSegmentation()
{
  std::string folderPathSeg = m_tempSegDir + getNextFolderName() + "/";
  itk::FileTools::CreateDirectory(folderPathSeg);

  const std::string savePath = folderPathSeg + m_SegC->GetName() + ".dcm";
  //const std::string mimeType = mitk::IOMimeTypes::DICOM_MIMETYPE_NAME();
  //mitk::IOUtil::Save(m_SegC->GetData(), mimeType, savePath);

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
  if (m_SegA || m_SegB || m_SegC) {
    QMessageBox::warning(nullptr, tr("Clean dicom folder"), tr("Please remove the data in data storage before cleaning the download folder"));
    return;
  }

  //std::experimental::filesystem::remove_all(m_downloadBaseDir);
  itk::FileTools::CreateDirectory(m_downloadBaseDir);
}
