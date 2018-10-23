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
#include <QmitkNewSegmentationDialog.h>
#include <mitkSegTool2D.h>
#include <mitkToolManagerProvider.h>
#include <mitkOverwriteSliceImageFilter.h>
#include <mitkSegmentationInterpolationController.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>

#include <mitkDICOMQIIOMimeTypes.h>

const std::string SegmentationReworkView::VIEW_ID = "org.mitk.views.segmentationreworkview";

void SegmentationReworkView::SetFocus() {}

void SegmentationReworkView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Parent = parent;
  counter = 0;

  qRegisterMetaType< std::vector<std::string> >("std::vector<std::string>");

  //m_Controls.verticalWidget->setVisible(false);
  m_Controls.cleanDicomBtn->setVisible(false);
  m_Controls.individualWidget_2->setVisible(false);

  m_Controls.sliderWidget->setMinimum(1);
  m_Controls.sliderWidget->setMaximum(100);
  m_Controls.sliderWidget->setTickInterval(1);
  m_Controls.sliderWidget->setSingleStep(1);
  m_Controls.radioA->setChecked(true);

  connect(m_Controls.buttonUpload, &QPushButton::clicked, this, &SegmentationReworkView::UploadNewSegmentation);
  connect(m_Controls.buttonNewSeg, &QPushButton::clicked, this, &SegmentationReworkView::CreateNewSegmentationC);
  connect(m_Controls.cleanDicomBtn, &QPushButton::clicked, this, &SegmentationReworkView::CleanDicomFolder);
  connect(m_Controls.restartConnection, &QPushButton::clicked, this, &SegmentationReworkView::RestartConnection);
  connect(m_Controls.checkIndiv, &QCheckBox::stateChanged, this, &SegmentationReworkView::OnIndividualCheckChange);
  connect(m_Controls.sliderWidget, &ctkSliderWidget::valueChanged, this, &SegmentationReworkView::OnSliderWidgetChanged);

  m_DownloadBaseDir = mitk::IOUtil::GetTempPathA() + "segrework";
  MITK_INFO << "using download base dir: " << m_DownloadBaseDir;
  m_UploadBaseDir = mitk::IOUtil::GetTempPathA() + "uploadSeg";

  if (!itksys::SystemTools::FileIsDirectory(m_DownloadBaseDir))
  {
    itk::FileTools::CreateDirectory(m_DownloadBaseDir);
  }

  if (!itksys::SystemTools::FileIsDirectory(m_UploadBaseDir))
  {
    itk::FileTools::CreateDirectory(m_UploadBaseDir);
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
  m_HttpHandler->SetGetImageSegCallback(std::bind(&SegmentationReworkView::RESTGetCallback, this, std::placeholders::_1));
  m_HttpHandler->SetGetAddSeriesCallback(std::bind(&SegmentationReworkView::RESTGetCallbackGeneric, this, std::placeholders::_1));
  m_HttpHandler->Open().wait();

  MITK_INFO << "Listening for requests at: " << utility::conversions::to_utf8string(address);

  //utility::string_t pacsURL = U("http://jip-dktk/dcm4chee-arc/aets/DCM4CHEE");
  utility::string_t pacsURL = U("http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE");
  m_DICOMWeb = new mitk::DICOMWeb(pacsURL);
  MITK_INFO << "requests to pacs are sent to: " << utility::conversions::to_utf8string(pacsURL);
  m_Controls.dcm4cheeURL->setText({ (utility::conversions::to_utf8string(pacsURL).c_str()) });
}

void SegmentationReworkView::OnSliderWidgetChanged(double value)
{
  std::map<double, double>::iterator it;
  unsigned int count = 0;
  for (it = m_ScoreMap.begin(); it != m_ScoreMap.end(); it++)
  {
    if (it->second < value)
    {
      count++;
    }
  }
  QString labelsToDelete = "slices to delete: " + QString::number(count);
  m_Controls.slicesToDeleteLabel->setText(labelsToDelete);
}



void SegmentationReworkView::RestartConnection()
{
  auto host = m_Controls.dcm4cheeHostValue->text();
  std::string url = host.toStdString() + "/dcm4chee-arc/aets/DCM4CHEE";

  m_DICOMWeb = new mitk::DICOMWeb(utility::conversions::to_string_t(url));
  MITK_INFO << "requests to pacs are sent to: " << url;
  m_Controls.dcm4cheeURL->setText({ (utility::conversions::to_utf8string(url).c_str()) });
}

void SegmentationReworkView::OnIndividualCheckChange(int state)
{
  if(state == Qt::Unchecked)
  {
    m_Controls.individualWidget_2->setVisible(false);
  }
  else if (state == Qt::Checked)
  {
    m_Controls.individualWidget_2->setVisible(true);
  }
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
    m_DICOMWeb->QuidoRSInstances(seriesInstancesParams).then([=](web::json::value jsonResult) 
    {

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

      std::string folderPathSeries = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) +"/";

      std::string pathSegA = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) + "/";
      std::string pathSegB = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) + "/";

      auto folderPathSegA = utility::conversions::to_string_t(pathSegA);
      auto folderPathSegB = utility::conversions::to_string_t(pathSegB);

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
        InvokeLoadData(filePathList);
      });

      ParamMap seriesInstancesParams;

      seriesInstancesParams.insert((ParamMap::value_type({ "StudyInstanceUID" }, dto.studyUID)));
      seriesInstancesParams.insert((ParamMap::value_type({ "SeriesInstanceUID" }, segSeriesUIDA)));

      m_DICOMWeb->QuidoRSInstances(seriesInstancesParams).then([=](web::json::value jsonResult) {
        auto seriesDescription = QString(utility::conversions::to_utf8string(jsonResult[0][U("0008103E")][U("Value")][0].as_string()).c_str());
        m_Controls.labelSegA->setText(m_Controls.labelSegA->text() + seriesDescription.append(" A"));
      });

      seriesInstancesParams["SeriesInstanceUID"] = segSeriesUIDB;
      m_DICOMWeb->QuidoRSInstances(seriesInstancesParams).then([=](web::json::value jsonResult) {
        auto seriesDescription = QString(utility::conversions::to_utf8string(jsonResult[0][U("0008103E")][U("Value")][0].as_string()).c_str());
        m_Controls.labelSegB->setText(m_Controls.labelSegB->text() + seriesDescription.append(" B"));
      });


    });

  }
  catch (mitk::Exception&  e) {
    MITK_ERROR << e.what();
  }
}

void SegmentationReworkView::RESTGetCallbackGeneric(const SegmentationReworkREST::DicomDTO &dto)
{
  std::vector<pplx::task<std::string>> tasks;

  if (dto.seriesUIDList.size() > 0) {
    for (std::string segSeriesUID : dto.seriesUIDList)
    {
      std::string folderPathSeries = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) + "/";
      try {
        auto seriesTask = m_DICOMWeb->WadoRS(utility::conversions::to_string_t(folderPathSeries), dto.studyUID, segSeriesUID);
        tasks.push_back(seriesTask);
      } catch (const mitk::Exception &exception) 
      {
        MITK_INFO << exception.what();
        return;
      }
    }
  }

  try 
  {
    auto joinTask = pplx::when_all(begin(tasks), end(tasks));
    auto filePathList = joinTask.then([&](std::vector<std::string> filePathList) { InvokeLoadData(filePathList); });
  }
  catch (const mitk::Exception& exception) 
  {
    MITK_INFO << exception.what();
    return;
  }
}

void SegmentationReworkView::RESTGetCallback(const SegmentationReworkREST::DicomDTO &dto) 
{
  std::string folderPathSeries = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) + "/";

  MITK_INFO << folderPathSeries;

  std::string pathSeg = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_DownloadBaseDir) + "/";
  auto folderPathSeg = utility::conversions::to_string_t(pathSeg);

  MITK_INFO << pathSeg;

  try {
    std::vector<pplx::task<std::string>> tasks;
    auto imageSeriesTask = m_DICOMWeb->WadoRS(utility::conversions::to_string_t(folderPathSeries), dto.studyUID, dto.imageSeriesUID);

    tasks.push_back(imageSeriesTask);

    if (dto.seriesUIDList.size() > 0) {
      for (std::string segSeriesUID : dto.seriesUIDList)
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
  catch (const mitk::Exception &exception) {
    MITK_INFO << exception.what();
  }
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
  std::string label = "similarity graph";
  if (m_Controls.chartWidget->GetData()->size() > 0)
  {
    m_Controls.chartWidget->Clear();
  }

  double sliceIndex = sliceMinStart;
  for (double score : simScoreArray)
  {
    m_ScoreMap.insert(std::map<double, double>::value_type(sliceIndex, score));
    sliceIndex++;
  }

  m_Controls.chartWidget->AddData2D(m_ScoreMap, label);
  m_Controls.chartWidget->SetChartType(label, QmitkChartWidget::ChartType::line);
  m_Controls.chartWidget->SetXAxisLabel("slice number");
  m_Controls.chartWidget->SetYAxisLabel("similarity in percent");
}

void SegmentationReworkView::UploadNewSegmentation()
{
  std::string folderPathSeg = mitk::IOUtil::CreateTemporaryDirectory("XXXXXX", m_UploadBaseDir) + "/";

  const std::string savePath = folderPathSeg + m_SegC->GetName() + ".dcm";
  const std::string mimeType = mitk::MitkDICOMQIIOMimeTypes::DICOMSEG_MIMETYPE_NAME();
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

std::vector<unsigned int> SegmentationReworkView::CreateSegmentation(mitk::Image::Pointer baseSegmentation, double threshold)
{
  MITK_INFO << "handle individual segmentation creation";
  std::map<double, double>::iterator it;

  std::vector<unsigned int> sliceIndices;

  unsigned int count = 0;
  for (it = m_ScoreMap.begin(); it != m_ScoreMap.end(); it++)
  {
    if (it->second < threshold)
    {
      auto index = it->first;
      try 
      {
        mitk::ImagePixelWriteAccessor<unsigned short, 3> imageAccessor(baseSegmentation);
        for (unsigned int x = 0; x < baseSegmentation->GetDimension(0); x++)
        {
          for (unsigned int y = 0; y < baseSegmentation->GetDimension(1); y++)
          {
            imageAccessor.SetPixelByIndex({ { x, y, int(index) } }, 0);
          }
        }
      }
      catch (mitk::Exception& e) {
        MITK_ERROR << e.what();
      }

      count++;
      sliceIndices.push_back(index);
      MITK_INFO << "slice " << it->first <<  " removed ";
	   }
  }
  MITK_INFO << "slices deleted " << count;
  return sliceIndices;
}

void SegmentationReworkView::InterpolateSegmentation(mitk::Image::Pointer baseSegmentation, std::vector<unsigned int> sliceIndices)
{
  mitk::SegmentationInterpolationController::Pointer interpolator = mitk::SegmentationInterpolationController::New();
  mitk::OverwriteSliceImageFilter::Pointer overwriteFilter = mitk::OverwriteSliceImageFilter::New();
  mitk::SliceNavigationController::Pointer navigationController = mitk::SliceNavigationController::New();
  navigationController->SetInputWorldTimeGeometry(baseSegmentation->GetTimeGeometry());
  navigationController->Update(mitk::SliceNavigationController::Axial);

  interpolator->SetSegmentationVolume(baseSegmentation);
  interpolator->SetReferenceVolume(dynamic_cast<mitk::Image*>(m_Image->GetData()));
  overwriteFilter->SetInput(baseSegmentation);

  MITK_INFO << "start with interpolation ";
  
  for (auto index : sliceIndices)
  {
    // look for the center within a slide
    itk::Index<3> centerPoint = { { baseSegmentation->GetDimension(0)/2, baseSegmentation->GetDimension(1)/2, index } };
    mitk::Point3D pointMM;
    baseSegmentation->GetTimeGeometry()->GetGeometryForTimeStep(0)->IndexToWorld(centerPoint, pointMM);
    navigationController->SelectSliceByPoint(pointMM);
    auto plane = navigationController->GetCurrentPlaneGeometry();
    unsigned int sliceDimension = 2;
    mitk::Image::Pointer interpolatedSlice = interpolator->Interpolate(sliceDimension, index, plane, 0);

    if (interpolatedSlice) {
      MITK_INFO << "add interpolated slice " << index;
      vtkSmartPointer<mitkVtkImageOverwrite> reslicer = vtkSmartPointer<mitkVtkImageOverwrite>::New();
      reslicer->SetInputSlice(interpolatedSlice->GetSliceData()->GetVtkImageAccessor(interpolatedSlice)->GetVtkImageData());
      reslicer->SetOverwriteMode(true);
      reslicer->Modified();
      mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslicer);
      extractor->SetInput(baseSegmentation);
      extractor->SetTimeStep(0);
      extractor->SetWorldGeometry(plane);
      extractor->SetVtkOutputRequest(true);
      extractor->SetResliceTransformByGeometry(baseSegmentation->GetTimeGeometry()->GetGeometryForTimeStep(0));
      extractor->Modified();
      extractor->Update();
    }
    else {
      MITK_INFO << "requested interpolation was null";
    }
  }

  MITK_INFO << "finished with interpolation ";
}

void SegmentationReworkView::CreateNewSegmentationC() 
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->InitializeTools();
  toolManager->SetReferenceData(m_Image);
  
  mitk::Image::Pointer baseImage;
  if (m_Controls.radioA->isChecked()) {
    baseImage = dynamic_cast<mitk::Image*>(m_SegA->GetData())->Clone();
  }
  else if (m_Controls.radioB->isChecked()) {
    baseImage = dynamic_cast<mitk::Image*>(m_SegB->GetData())->Clone();
  }

  if (m_Controls.checkIndiv->isChecked())
  {
    auto sliceIndices = CreateSegmentation(baseImage, m_Controls.sliderWidget->value());
    //InterpolateSegmentation(baseImage, sliceIndices);
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

      auto referencedImages = m_Image->GetData()->GetProperty("files");
      m_SegC->GetData()->SetProperty("referenceFiles", referencedImages);

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

  //std::experimental::filesystem::remove_all(m_DownloadBaseDir);
  // TODO : use POCO
  //itk::FileTools::CreateDirectory(m_DownloadBaseDir);
}
