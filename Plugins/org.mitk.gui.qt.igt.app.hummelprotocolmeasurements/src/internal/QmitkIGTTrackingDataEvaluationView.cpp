/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <algorithm>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkIGTTrackingDataEvaluationView.h"

// Qt
#include <QMessageBox>
#include <qfiledialog.h>
#include <qstringlist.h>

// MITK
#include "mitkNavigationDataCSVSequentialPlayer.h"
#include <mitkNavigationDataRecorderDeprecated.h>
#include <mitkQuaternionAveraging.h>
#include <mitkTransform.h>
#include <mitkStaticIGTHelperFunctions.h>
#include <mitkNodePredicateDataType.h>


//ITK
#include <itksys/SystemTools.hxx>

//VNL
#include <vnl/vnl_vector.h>

//vtk headers
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkLandmarkTransform.h>

const std::string QmitkIGTTrackingDataEvaluationView::VIEW_ID = "org.mitk.views.igttrackingdataevaluation";

QmitkIGTTrackingDataEvaluationView::QmitkIGTTrackingDataEvaluationView()
  : m_Controls(nullptr)
  , m_scalingfactor(1)
{
  m_CSVtoXMLInputFilenameVector = std::vector<std::string>();
  m_CSVtoXMLOutputFilenameVector = std::vector<std::string>();
}

QmitkIGTTrackingDataEvaluationView::~QmitkIGTTrackingDataEvaluationView()
{
}

void QmitkIGTTrackingDataEvaluationView::CreateQtPartControl(QWidget *parent)
{
  // build up qt view, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkIGTTrackingDataEvaluationViewControls;
    m_Controls->setupUi(parent);

    connect(m_Controls->m_LoadInputFileList, SIGNAL(clicked()), this, SLOT(OnLoadFileList()));
    connect(m_Controls->m_StartEvaluation, SIGNAL(clicked()), this, SLOT(OnEvaluateData()));
    connect(m_Controls->m_AddToCurrentList, SIGNAL(clicked()), this, SLOT(OnAddToCurrentList()));
    connect(m_Controls->m_GeneratePointSetOfMeanPositions, SIGNAL(clicked()), this, SLOT(OnGeneratePointSet()));
    connect(m_Controls->m_GenerateRotationLines, SIGNAL(clicked()), this, SLOT(OnGenerateRotationLines()));
    connect(m_Controls->m_GeneratePointSet, SIGNAL(clicked()), this, SLOT(OnGenerateGroundTruthPointSet()));
    connect(m_Controls->m_Convert, SIGNAL(clicked()), this, SLOT(OnConvertCSVtoXMLFile()));
    connect(m_Controls->m_loadCSVtoXMLInputList, SIGNAL(clicked()), this, SLOT(OnCSVtoXMLLoadInputList()));
    connect(m_Controls->m_loadCSVtoXMLOutputList, SIGNAL(clicked()), this, SLOT(OnCSVtoXMLLoadOutputList()));
    connect(m_Controls->m_OrientationCalculationGenerateReference, SIGNAL(clicked()), this, SLOT(OnOrientationCalculation_CalcRef()));
    connect(m_Controls->m_OrientationCalculationWriteOrientationsToFile, SIGNAL(clicked()), this, SLOT(OnOrientationCalculation_CalcOrientandWriteToFile()));
    connect(m_Controls->m_GeneratePointSetsOfSinglePositions, SIGNAL(clicked()), this, SLOT(OnGeneratePointSetsOfSinglePositions()));
    connect(m_Controls->m_StartEvaluationAll, SIGNAL(clicked()), this, SLOT(OnEvaluateDataAll()));
    connect(m_Controls->m_GridMatching, SIGNAL(clicked()), this, SLOT(OnPerfomGridMatching()));
    connect(m_Controls->m_ComputeRotation, SIGNAL(clicked()), this, SLOT(OnComputeRotation()));

    //initialize data storage combo boxes
    m_Controls->m_ReferencePointSetComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ReferencePointSetComboBox->SetAutoSelectNewItems(true);
    m_Controls->m_ReferencePointSetComboBox->SetPredicate(mitk::NodePredicateDataType::New("PointSet"));
    m_Controls->m_MeasurementPointSetComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MeasurementPointSetComboBox->SetAutoSelectNewItems(true);
    m_Controls->m_MeasurementPointSetComboBox->SetPredicate(mitk::NodePredicateDataType::New("PointSet"));
  }
}

void QmitkIGTTrackingDataEvaluationView::OnComputeRotation()
{
  //Get all data from UI
  auto EvaluationDataCollection = GetAllDataFromUIList();
  //Compute mean Quaternions
  auto OrientationVector = GetMeanOrientationsOfAllData(EvaluationDataCollection);

  //Compute Rotations

  itk::Vector<double> rotationVec;
  //adapt for Aurora 5D tools: [0,0,1000]
  rotationVec[0] = m_Controls->m_rotVecX->value(); //X
  rotationVec[1] = m_Controls->m_rotVecY->value(); //Y
  rotationVec[2] = m_Controls->m_rotVecZ->value(); //Z

  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> allOrientationErrors;
  for (std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError>::size_type i = 0; i < OrientationVector.size() - 1; ++i)
  {
    double AngleBetweenTwoQuaternions = mitk::StaticIGTHelperFunctions::GetAngleBetweenTwoQuaterions(OrientationVector.at(i), OrientationVector.at(i+1), rotationVec);
    double AngularError = fabs(AngleBetweenTwoQuaternions - 11.25);
    std::stringstream description;
    description << "Rotation Error ROT" << (i + 1) << " / ROT" << (i + 2);
    allOrientationErrors.push_back({ AngularError, description.str() });
    MITK_INFO << description.str() << ": " << AngularError;
  }

  //compute statistics
  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> orientationErrorStatistics;
  orientationErrorStatistics = mitk::HummelProtocolEvaluation::ComputeStatistics(allOrientationErrors);
  MITK_INFO << "## Rotation error statistics: ##";
  for (auto stat : orientationErrorStatistics) { MITK_INFO << stat.description << ": " << stat.distanceError; }

  //write results to file
  allOrientationErrors.insert(allOrientationErrors.end(), orientationErrorStatistics.begin(), orientationErrorStatistics.end());
  allOrientationErrors.push_back({rotationVec[0],"Rot Vector [x]"});
  allOrientationErrors.push_back({rotationVec[1], "Rot Vector [y]"});
  allOrientationErrors.push_back({rotationVec[2], "Rot Vector [z]"});
  std::stringstream filenameOrientationStat;
  filenameOrientationStat << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".orientationStatistics.csv";
  MITK_INFO << "Writing output to file " << filenameOrientationStat.str();
  writeToFile(filenameOrientationStat.str(), allOrientationErrors);
}

void QmitkIGTTrackingDataEvaluationView::OnPerfomGridMatching()
{
  mitk::PointSet::Pointer reference = dynamic_cast<mitk::PointSet*>(m_Controls->m_ReferencePointSetComboBox->GetSelectedNode()->GetData());
  mitk::PointSet::Pointer measurement = dynamic_cast<mitk::PointSet*>(m_Controls->m_MeasurementPointSetComboBox->GetSelectedNode()->GetData());
  //convert point sets to vtk poly data
  vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
  for (int i = 0; i<reference->GetSize(); i++)
  {
    double point[3] = { reference->GetPoint(i)[0], reference->GetPoint(i)[1], reference->GetPoint(i)[2] };
    sourcePoints->InsertNextPoint(point);
    double point_targets[3] = { measurement->GetPoint(i)[0], measurement->GetPoint(i)[1], measurement->GetPoint(i)[2] };
    targetPoints->InsertNextPoint(point_targets);
  }
  //compute transform
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
  transform->SetSourceLandmarks(sourcePoints);
  transform->SetTargetLandmarks(targetPoints);
  transform->SetModeToRigidBody();
  transform->Modified();
  transform->Update();
  //compute FRE of transform
  double FRE = mitk::StaticIGTHelperFunctions::ComputeFRE(reference, measurement, transform);
  MITK_INFO << "FRE after grid matching: " + QString::number(FRE) + " mm";
  //convert from vtk to itk data types
  itk::Matrix<float, 3, 3> rotationFloat = itk::Matrix<float, 3, 3>();
  itk::Vector<float, 3> translationFloat = itk::Vector<float, 3>();
  itk::Matrix<double, 3, 3> rotationDouble = itk::Matrix<double, 3, 3>();
  itk::Vector<double, 3> translationDouble = itk::Vector<double, 3>();

  vtkSmartPointer<vtkMatrix4x4> m = transform->GetMatrix();
  for (int k = 0; k<3; k++) for (int l = 0; l<3; l++)
  {
    rotationFloat[k][l] = m->GetElement(k, l);
    rotationDouble[k][l] = m->GetElement(k, l);

  }
  for (int k = 0; k<3; k++)
  {
    translationFloat[k] = m->GetElement(k, 3);
    translationDouble[k] = m->GetElement(k, 3);
  }
  //create affine transform 3D
  mitk::AffineTransform3D::Pointer mitkTransform = mitk::AffineTransform3D::New();
  mitkTransform->SetMatrix(rotationDouble);
  mitkTransform->SetOffset(translationDouble);
  mitk::NavigationData::Pointer transformNavigationData = mitk::NavigationData::New(mitkTransform);
  m_Controls->m_ReferencePointSetComboBox->GetSelectedNode()->GetData()->GetGeometry()->SetIndexToWorldTransform(mitkTransform);
  m_Controls->m_ReferencePointSetComboBox->GetSelectedNode()->GetData()->GetGeometry()->Modified();

  //write to file

  std::stringstream filename;
  filename << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".GridMatchingResult.csv";
  MITK_INFO << "Writing output to file " << filename.str();
  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> FRE_Error;
  FRE_Error.push_back({ FRE, "FRE after grid matching [mm]" });
  writeToFile(filename.str(), FRE_Error);
}

void QmitkIGTTrackingDataEvaluationView::OnOrientationCalculation_CalcRef()
{
  if (m_FilenameVector.size() != 3)
  {
    MessageBox("Need exactly three points as reference, aborting!");
    return;
  }

  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector[i]);

    //check if the stream is valid and skip file if not

    //create evaluation filter
    mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();

    //connect pipeline
    for (unsigned int j = 0; j < myPlayer->GetNumberOfOutputs(); ++j)
      myEvaluationFilter->SetInput(j, myPlayer->GetOutput(j));

    //update pipline until number of samples is reached
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); ++j)
      myEvaluationFilter->Update();

    //store mean position as reference
    switch (i)
    {
    case 0:
      m_RefPoint1 = myEvaluationFilter->GetPositionMean(0);
      break;
    case 1:
      m_RefPoint2 = myEvaluationFilter->GetPositionMean(0);
      break;
    case 2:
      m_RefPoint3 = myEvaluationFilter->GetPositionMean(0);
      break;
    }
  }
  MessageBox("Created Reference!");
}

void QmitkIGTTrackingDataEvaluationView::OnOrientationCalculation_CalcOrientandWriteToFile()
{
  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //open file header
    QString outputname = QString(m_FilenameVector.at(i).c_str()) + "_orientationFile.csv";
    m_CurrentWriteFile.open(outputname.toStdString().c_str(), std::ios::out);
    if (m_CurrentWriteFile.bad())
    {
      MessageBox("Error: Can't open output file!");
      return;
    }

    //write header to file
    m_CurrentWriteFile << "Nr;Calypso_Time;Valid_Reference;MeasureTool_Measurement-Tool[x];MeasureTool_Measurement-Tool[y];MeasureTool_Measurement-Tool[z];MeasureTool_Measurement-Tool[qx];MeasureTool_Measurement-Tool[qy];MeasureTool_Measurement-Tool[qz];MeasureTool_Measurement-Tool[qr]\n";

    //update pipeline until number of samples is reached
    int step = 0;
    mitk::Point3D point1, point2, point3;
    mitk::Quaternion current_orientation;

    for (int j = 0; !myPlayer->IsAtEnd(); j++)
    {
      myPlayer->Update();
      mitk::NavigationData::Pointer currentNavData = myPlayer->GetOutput(0);
      switch (step)
      {
      case 0:
        step++;
        point1 = currentNavData->GetPosition();
        break;
      case 1:
        step++;
        point2 = currentNavData->GetPosition();
        break;
      case 2:
        step = 0;
        point3 = currentNavData->GetPosition();

        //compute transform from reference to current points
        if (point1[0] == 0 &&
          point1[1] == 0 &&
          point1[2] == 0 &&
          point2[0] == 0 &&
          point2[1] == 0 &&
          point2[2] == 0 &&
          point3[0] == 0 &&
          point3[1] == 0 &&
          point3[2] == 0
          ) current_orientation.fill(0);
        else
        {
          vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
          vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
          double sourcepoint1[3] = { point1[0], point1[1], point1[2] };
          double sourcepoint2[3] = { point2[0], point2[1], point2[2] };
          double sourcepoint3[3] = { point3[0], point3[1], point3[2] };
          sourcePoints->InsertNextPoint(sourcepoint1);
          sourcePoints->InsertNextPoint(sourcepoint2);
          sourcePoints->InsertNextPoint(sourcepoint3);
          vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
          double targetpoint1[3] = { m_RefPoint1[0], m_RefPoint1[1], m_RefPoint1[2] };
          double targetpoint2[3] = { m_RefPoint2[0], m_RefPoint2[1], m_RefPoint2[2] };
          double targetpoint3[3] = { m_RefPoint3[0], m_RefPoint3[1], m_RefPoint3[2] };
          targetPoints->InsertNextPoint(targetpoint1);
          targetPoints->InsertNextPoint(targetpoint2);
          targetPoints->InsertNextPoint(targetpoint3);

          transform->SetSourceLandmarks(sourcePoints);
          transform->SetTargetLandmarks(targetPoints);
          transform->Modified();
          transform->Update();

          mitk::Transform::Pointer newTransform = mitk::Transform::New();
          newTransform->SetMatrix(transform->GetMatrix());
          current_orientation = newTransform->GetOrientation();

          //add pointset with the three positions
          if ((j > 15) && (j < 18))
          {
            mitk::DataNode::Pointer newNode = mitk::DataNode::New();
            mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
            newPointSet->InsertPoint(0, point1);
            newPointSet->InsertPoint(1, point2);
            newPointSet->InsertPoint(2, point3);
            QString name = QString(m_FilenameVector.at(i).c_str());
            newNode->SetName(name.toStdString().c_str());
            newNode->SetData(newPointSet);
            newNode->SetFloatProperty("pointsize", 0.1);
            this->GetDataStorage()->Add(newNode);
          }
        }

        break;
      }
      m_CurrentWriteFile << i << ";";
      m_CurrentWriteFile << currentNavData->GetTimeStamp() << ";"; //IMPORTANT: change to GetIGTTimeStamp in new version!
      m_CurrentWriteFile << "true;";
      m_CurrentWriteFile << currentNavData->GetPosition()[0] << ";";
      m_CurrentWriteFile << currentNavData->GetPosition()[1] << ";";
      m_CurrentWriteFile << currentNavData->GetPosition()[2] << ";";
      m_CurrentWriteFile << current_orientation.x() << ";";
      m_CurrentWriteFile << current_orientation.y() << ";";
      m_CurrentWriteFile << current_orientation.z() << ";";
      m_CurrentWriteFile << current_orientation.r() << ";";
      m_CurrentWriteFile << "\n";
    }
    //close output file
    m_CurrentWriteFile.close();
  }
  MessageBox("Finished!");
}

void QmitkIGTTrackingDataEvaluationView::OnAddToCurrentList()
{
  //read in files
  QStringList files = QFileDialog::getOpenFileNames(nullptr, "Select one or more files to open", "/", "CSV (*.csv)");
  if (files.isEmpty()) return;

  for (int i = 0; i < files.size(); i++)
  {
    std::string tmp = files.at(i).toStdString().c_str();
    m_FilenameVector.push_back(tmp);
  }

  //fill list at GUI
  m_Controls->m_FileList->clear();
  for (unsigned int i = 0; i < m_FilenameVector.size(); i++) { new QListWidgetItem(tr(m_FilenameVector.at(i).c_str()), m_Controls->m_FileList); }
}

void QmitkIGTTrackingDataEvaluationView::OnLoadFileList()
{
  m_FilenameVector = std::vector<std::string>();
  m_FilenameVector.clear();
  OnAddToCurrentList();
}

void QmitkIGTTrackingDataEvaluationView::OnEvaluateDataAll()
{
  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> results5cm, results15cm, results30cm, resultsAccum;
  mitk::HummelProtocolEvaluation::HummelProtocolMeasurementVolume volume;
  if (m_Controls->m_standardVolume->isChecked())
  {
    volume = mitk::HummelProtocolEvaluation::standard;
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_PointSetMeanPositions, volume, results5cm);
    mitk::HummelProtocolEvaluation::Evaluate15cmDistances(m_PointSetMeanPositions, volume, results15cm);
    mitk::HummelProtocolEvaluation::Evaluate30cmDistances(m_PointSetMeanPositions, volume, results30cm);
    mitk::HummelProtocolEvaluation::EvaluateAccumulatedDistances(m_PointSetMeanPositions, volume, resultsAccum);
  }
  else if (m_Controls->m_smallVolume->isChecked())
  {
    volume = mitk::HummelProtocolEvaluation::small;
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_PointSetMeanPositions, volume, results5cm);
  }
  else if (m_Controls->m_mediumVolume->isChecked())
  {
    volume = mitk::HummelProtocolEvaluation::medium;
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_PointSetMeanPositions, volume, results5cm);
  }


  //write results to file
  std::stringstream filename5cm;
  filename5cm << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".results5cm.csv";
  MITK_INFO << "Writing output to file " << filename5cm.str();
  writeToFile(filename5cm.str(), results5cm);

  std::stringstream filename15cm;
  filename15cm << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".results15cm.csv";
  MITK_INFO << "Writing output to file " << filename15cm.str();
  writeToFile(filename15cm.str(), results15cm);

  std::stringstream filename30cm;
  filename30cm << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".results30cm.csv";
  MITK_INFO << "Writing output to file " << filename30cm.str();
  writeToFile(filename30cm.str(), results30cm);

  std::stringstream filenameAccum;
  filenameAccum << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".resultsAccumDist.csv";
  MITK_INFO << "Writing output to file " << filenameAccum.str();
  writeToFile(filenameAccum.str(), resultsAccum);
}

void QmitkIGTTrackingDataEvaluationView::OnEvaluateData()
{
  //open output file
  m_CurrentWriteFile.open(std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str(), std::ios::out);
  if (m_CurrentWriteFile.bad())
  {
    MessageBox("Error: Can't open output file!");
    return;
  }

  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> jitterValues;

  //write output file header
  WriteHeader();

  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //create evaluation filter
    mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();

    //connect pipeline
    for (unsigned int j = 0; j < myPlayer->GetNumberOfOutputs(); ++i) { myEvaluationFilter->SetInput(j, myPlayer->GetOutput(j)); }

    if (myPlayer->GetNumberOfSnapshots() < m_Controls->m_NumberOfSamples->value())
    {
      MITK_WARN << "Number of snapshots (" << myPlayer->GetNumberOfSnapshots() << ") smaller than number of samples to evaluate (" << m_Controls->m_NumberOfSamples->value() << ") ! Cannot proceed!";
      return;
    }

    //update pipline until number of samples is reached
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); j++)
      myEvaluationFilter->Update();

    //store all jitter values in separate vector for statistics
    jitterValues.push_back({ myEvaluationFilter->GetPositionErrorRMS(0), "RMS" });

    //write result to output file
    WriteDataSet(myEvaluationFilter, m_FilenameVector.at(i));
  }

  //close output file for single data
  m_CurrentWriteFile.close();

  //compute statistics
  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> jitterStatistics = mitk::HummelProtocolEvaluation::ComputeStatistics(jitterValues);
  MITK_INFO << "## Jitter (RMS) statistics: ##";
  for (auto jitterStat : jitterStatistics) {MITK_INFO << jitterStat.description << ": " << jitterStat.distanceError;}

  //write statistic results to separate file
  std::stringstream filenameJitterStat;
  filenameJitterStat << std::string(m_Controls->m_OutputFilename->text().toUtf8()).c_str() << ".resultsJitterStatistics.csv";
  MITK_INFO << "Writing output to file " << filenameJitterStat.str();
  writeToFile(filenameJitterStat.str(), jitterStatistics);

  //calculate angles if option is on
  if (m_Controls->m_settingDifferenceAngles->isChecked() || m_Controls->m_DifferencesSLERP->isChecked()) CalculateDifferenceAngles();

  MessageBox("Finished!");
}

void QmitkIGTTrackingDataEvaluationView::OnGeneratePointSetsOfSinglePositions()
{
  m_scalingfactor = m_Controls->m_ScalingFactor->value();

  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create point set for this file
    mitk::PointSet::Pointer thisPointSet = mitk::PointSet::New();

    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //update pipline until number of samlples is reached and store every single point
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); j++)
    {
      myPlayer->Update();
      mitk::Point3D thisPoint = myPlayer->GetOutput()->GetPosition();
      thisPoint[0] *= m_scalingfactor;
      thisPoint[1] *= m_scalingfactor;
      thisPoint[2] *= m_scalingfactor;
      thisPointSet->InsertPoint(j, thisPoint);
    }

    //add point set to data storage
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    QString name = this->m_Controls->m_prefix->text() + QString("PointSet_of_All_Positions_") + QString::number(i);
    newNode->SetName(name.toStdString());
    newNode->SetData(thisPointSet);
    this->GetDataStorage()->Add(newNode);
  }
}

void QmitkIGTTrackingDataEvaluationView::OnGeneratePointSet()
{
  m_scalingfactor = m_Controls->m_ScalingFactor->value();

  mitk::PointSet::Pointer generatedPointSet = mitk::PointSet::New();

  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //create evaluation filter
    mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();

    //connect pipeline
    for (unsigned int j = 0; j < myPlayer->GetNumberOfOutputs(); ++j) { myEvaluationFilter->SetInput(j, myPlayer->GetOutput(j)); }

    //update pipline until number of samlples is reached
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); ++j) { myEvaluationFilter->Update(); }

    //add mean position to point set
    mitk::Point3D meanPos = myEvaluationFilter->GetPositionMean(0);
    if (m_scalingfactor != 1)
    {
      meanPos[0] *= m_scalingfactor;
      meanPos[1] *= m_scalingfactor;
      meanPos[2] *= m_scalingfactor;
    }
    generatedPointSet->InsertPoint(i, meanPos);
  }

  //add point set to data storage
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  QString name = this->m_Controls->m_prefix->text() + "PointSet_of_Mean_Positions";
  newNode->SetName(name.toStdString());
  newNode->SetData(generatedPointSet);
  newNode->SetFloatProperty("pointsize", 5);
  this->GetDataStorage()->Add(newNode);
  m_PointSetMeanPositions = generatedPointSet;
}

void QmitkIGTTrackingDataEvaluationView::OnGenerateRotationLines()
{
  m_scalingfactor = m_Controls->m_ScalingFactor->value();

  //start loop and iterate through all files of list
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //create evaluation filter
    mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();

    //connect pipeline
    for (unsigned int j = 0; j < myPlayer->GetNumberOfOutputs(); ++j) { myEvaluationFilter->SetInput(j, myPlayer->GetOutput(j)); }

    //update pipline until number of samlples is reached
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); ++j)
      myEvaluationFilter->Update();

    //create line from mean pos to a second point which lies along the sensor (1,0,0 in tool coordinates for aurora)
    mitk::Point3D meanPos = myEvaluationFilter->GetPositionMean(0);
    if (m_scalingfactor != 1)
    {
      meanPos[0] *= m_scalingfactor;
      meanPos[1] *= m_scalingfactor;
      meanPos[2] *= m_scalingfactor;
    }
    mitk::Point3D secondPoint;
    mitk::Point3D thirdPoint;
    mitk::Point3D fourthPoint;

    mitk::FillVector3D(secondPoint, 2, 0, 0); //X
    vnl_vector<mitk::ScalarType> secondPointTransformed = myEvaluationFilter->GetQuaternionMean(0).rotation_matrix_transpose().transpose() * secondPoint.GetVnlVector() + meanPos.GetVnlVector();
    mitk::Point3D secondPointTransformedMITK;
    mitk::FillVector3D(secondPointTransformedMITK, secondPointTransformed[0], secondPointTransformed[1], secondPointTransformed[2]);

    mitk::FillVector3D(thirdPoint, 0, 4, 0); //Y
    vnl_vector<mitk::ScalarType> thirdPointTransformed = myEvaluationFilter->GetQuaternionMean(0).rotation_matrix_transpose().transpose() * thirdPoint.GetVnlVector() + meanPos.GetVnlVector();
    mitk::Point3D thirdPointTransformedMITK;
    mitk::FillVector3D(thirdPointTransformedMITK, thirdPointTransformed[0], thirdPointTransformed[1], thirdPointTransformed[2]);

    mitk::FillVector3D(fourthPoint, 0, 0, 6); //Z
    vnl_vector<mitk::ScalarType> fourthPointTransformed = myEvaluationFilter->GetQuaternionMean(0).rotation_matrix_transpose().transpose() * fourthPoint.GetVnlVector() + meanPos.GetVnlVector();
    mitk::Point3D fourthPointTransformedMITK;
    mitk::FillVector3D(fourthPointTransformedMITK, fourthPointTransformed[0], fourthPointTransformed[1], fourthPointTransformed[2]);

    mitk::PointSet::Pointer rotationLine = mitk::PointSet::New();
    rotationLine->InsertPoint(0, secondPointTransformedMITK);
    rotationLine->InsertPoint(1, meanPos);
    rotationLine->InsertPoint(2, thirdPointTransformedMITK);
    rotationLine->InsertPoint(3, meanPos);
    rotationLine->InsertPoint(4, fourthPointTransformedMITK);

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    QString nodeName = this->m_Controls->m_prefix->text() + "RotationLineNumber" + QString::number(i);
    newNode->SetName(nodeName.toStdString());
    newNode->SetData(rotationLine);
    newNode->SetBoolProperty("show contour", true);
    newNode->SetFloatProperty("pointsize", 0.5);
    this->GetDataStorage()->Add(newNode);
  }
}

void QmitkIGTTrackingDataEvaluationView::OnGenerateGroundTruthPointSet()
{
  mitk::PointSet::Pointer generatedPointSet = mitk::PointSet::New();
  int currentPointID = 0;
  mitk::Point3D currentPoint;
  mitk::FillVector3D(currentPoint, 0, 0, 0);
  for (int i = 0; i < m_Controls->m_PointNumber2->value(); i++)
  {
    for (int j = 0; j < m_Controls->m_PointNumber1->value(); j++)
    {
      generatedPointSet->InsertPoint(currentPointID, currentPoint);
      currentPointID++;
      currentPoint[1] += m_Controls->m_PointDistance->value();
    }
    currentPoint[1] = 0;
    currentPoint[2] += m_Controls->m_PointDistance->value();
  }
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  QString nodeName = "GroundTruthPointSet_" + QString::number(m_Controls->m_PointNumber1->value()) + "x" + QString::number(m_Controls->m_PointNumber2->value()) + "_(" + QString::number(m_Controls->m_PointDistance->value()) + "mm)";
  newNode->SetName(nodeName.toStdString());
  newNode->SetData(generatedPointSet);
  newNode->SetFloatProperty("pointsize", 5);
  this->GetDataStorage()->Add(newNode);
}

void QmitkIGTTrackingDataEvaluationView::OnConvertCSVtoXMLFile()
{
  if (m_Controls->m_ConvertSingleFile->isChecked())
  { //convert one file
    int lines = ConvertOneFile(this->m_Controls->m_InputCSV->text().toStdString(), this->m_Controls->m_OutputXML->text().toStdString());

    QString result = "Converted one file with" + QString::number(lines) + " data sets";
    MessageBox(result.toStdString());
  }
  else //converte file list
  {
    if (m_CSVtoXMLInputFilenameVector.empty() || m_CSVtoXMLOutputFilenameVector.empty())
    {
      MessageBox("Error: one list is not loaded!");
      return;
    }
    else if (m_CSVtoXMLInputFilenameVector.size() != m_CSVtoXMLOutputFilenameVector.size())
    {
      MessageBox("Error: lists do not have the same number of files!");
      return;
    }
    for (std::size_t i = 0; i < m_CSVtoXMLInputFilenameVector.size(); ++i)
    {
      ConvertOneFile(m_CSVtoXMLInputFilenameVector.at(i), m_CSVtoXMLOutputFilenameVector.at(i));
    }
    QString result = "Converted " + QString::number(m_CSVtoXMLInputFilenameVector.size()) + " files from file list!";
    MessageBox(result.toStdString());
  }
}

int QmitkIGTTrackingDataEvaluationView::ConvertOneFile(std::string inputFilename, std::string outputFilename)
{
  std::vector<mitk::NavigationData::Pointer> myNavigationDatas = GetNavigationDatasFromFile(inputFilename);
  mitk::NavigationDataRecorderDeprecated::Pointer myRecorder = mitk::NavigationDataRecorderDeprecated::New();
  myRecorder->SetFileName(outputFilename.c_str());
  mitk::NavigationData::Pointer input = mitk::NavigationData::New();
  if (m_Controls->m_ConvertCSV->isChecked()) myRecorder->SetOutputFormat(mitk::NavigationDataRecorderDeprecated::csv);
  myRecorder->AddNavigationData(input);
  myRecorder->StartRecording();
  for (std::size_t i = 0; i < myNavigationDatas.size(); ++i)
  {
    input->Graft(myNavigationDatas.at(i));
    myRecorder->Update();
  }
  myRecorder->StopRecording();
  return myNavigationDatas.size();
}

void QmitkIGTTrackingDataEvaluationView::OnCSVtoXMLLoadInputList()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Measurement Filename List"), "/", tr("All Files (*.*)"));
  if (filename.isNull()) return;

  m_CSVtoXMLInputFilenameVector = this->GetFileContentLineByLine(filename.toStdString());

  m_Controls->m_labelCSVtoXMLInputList->setText("READY");
}

void QmitkIGTTrackingDataEvaluationView::OnCSVtoXMLLoadOutputList()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Measurement Filename List"), "/", tr("All Files (*.*)"));
  if (filename.isNull()) return;

  m_CSVtoXMLOutputFilenameVector = this->GetFileContentLineByLine(filename.toStdString());

  m_Controls->m_labelCSVtoXMLOutputList->setText("READY");
}

void QmitkIGTTrackingDataEvaluationView::MessageBox(std::string s)
{
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
}

void QmitkIGTTrackingDataEvaluationView::WriteHeader()
{
  m_CurrentWriteFile << "Filename;";
  m_CurrentWriteFile << "N;";
  m_CurrentWriteFile << "N_invalid;";
  m_CurrentWriteFile << "Percentage_invalid;";

  if (m_Controls->m_settingPosMean->isChecked())
  {
    m_CurrentWriteFile << "Position_Mean[x];";
    m_CurrentWriteFile << "Position_Mean[y];";
    m_CurrentWriteFile << "Position_Mean[z];";
  }

  if (m_Controls->m_settingPosStabw->isChecked())
  {
    m_CurrentWriteFile << "Position_StandDev[x];";
    m_CurrentWriteFile << "Position_StandDev[y];";
    m_CurrentWriteFile << "Position_StandDev[z];";
  }

  if (m_Controls->m_settingPosSampleStabw->isChecked())
  {
    m_CurrentWriteFile << "Position_SampleStandDev[x];";
    m_CurrentWriteFile << "Position_SampleStandDev[y];";
    m_CurrentWriteFile << "Position_SampleStandDev[z];";
  }

  if (m_Controls->m_settingQuaternionMean->isChecked())
  {
    m_CurrentWriteFile << "Quaternion_Mean[qx];";
    m_CurrentWriteFile << "Quaternion_Mean[qy];";
    m_CurrentWriteFile << "Quaternion_Mean[qz];";
    m_CurrentWriteFile << "Quaternion_Mean[qr];";
  }

  if (m_Controls->m_settionQuaternionStabw->isChecked())
  {
    m_CurrentWriteFile << "Quaternion_StandDev[qx];";
    m_CurrentWriteFile << "Quaternion_StandDev[qy];";
    m_CurrentWriteFile << "Quaternion_StandDev[qz];";
    m_CurrentWriteFile << "Quaternion_StandDev[qr];";
  }

  if (m_Controls->m_settingPosErrorMean->isChecked()) m_CurrentWriteFile << "PositionError_Mean;";

  if (m_Controls->m_settingPosErrorStabw->isChecked()) m_CurrentWriteFile << "PositionError_StandDev;";

  if (m_Controls->m_settingPosErrorSampleStabw->isChecked()) m_CurrentWriteFile << "PositionError_SampleStandDev;";

  if (m_Controls->m_settingPosErrorRMS->isChecked()) m_CurrentWriteFile << "PositionError_RMS;";

  if (m_Controls->m_settingPosErrorMedian->isChecked()) m_CurrentWriteFile << "PositionError_Median;";

  if (m_Controls->m_settingPosErrorMinMax->isChecked())
  {
    m_CurrentWriteFile << "PositionError_Max;";
    m_CurrentWriteFile << "PositionError_Min;";
  }

  if (m_Controls->m_settingEulerMean->isChecked())
  {
    m_CurrentWriteFile << "Euler_tx;";
    m_CurrentWriteFile << "Euler_ty;";
    m_CurrentWriteFile << "Euler_tz;";
  }

  if (m_Controls->m_settingEulerRMS->isChecked())
  {
    m_CurrentWriteFile << "EulerErrorRMS (rad);";
    m_CurrentWriteFile << "EulerErrorRMS (grad);";
  }

  m_CurrentWriteFile << "\n";
}

void QmitkIGTTrackingDataEvaluationView::WriteDataSet(mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter, std::string dataSetName)
{
  if (myEvaluationFilter->GetNumberOfOutputs() == 0) m_CurrentWriteFile << "Error: no input \n";
  else
  {
    m_CurrentWriteFile << dataSetName << ";";
    m_CurrentWriteFile << myEvaluationFilter->GetNumberOfAnalysedNavigationData(0) << ";";
    m_CurrentWriteFile << myEvaluationFilter->GetNumberOfInvalidSamples(0) << ";";
    m_CurrentWriteFile << myEvaluationFilter->GetPercentageOfInvalidSamples(0) << ";";

    if (m_Controls->m_settingPosMean->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetPositionMean(0)[0] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionMean(0)[1] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionMean(0)[2] << ";";
    }

    if (m_Controls->m_settingPosStabw->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetPositionStandardDeviation(0)[0] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionStandardDeviation(0)[1] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionStandardDeviation(0)[2] << ";";
    }

    if (m_Controls->m_settingPosSampleStabw->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetPositionSampleStandardDeviation(0)[0] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionSampleStandardDeviation(0)[1] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionSampleStandardDeviation(0)[2] << ";";
    }

    if (m_Controls->m_settingQuaternionMean->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionMean(0).x() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionMean(0).y() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionMean(0).z() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionMean(0).r() << ";";
    }

    if (m_Controls->m_settionQuaternionStabw->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionStandardDeviation(0).x() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionStandardDeviation(0).y() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionStandardDeviation(0).z() << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetQuaternionStandardDeviation(0).r() << ";";
    }

    if (m_Controls->m_settingPosErrorMean->isChecked()) m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorMean(0) << ";";
    if (m_Controls->m_settingPosErrorStabw->isChecked()) m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorStandardDeviation(0) << ";";
    if (m_Controls->m_settingPosErrorSampleStabw->isChecked()) m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorSampleStandardDeviation(0) << ";";
    if (m_Controls->m_settingPosErrorRMS->isChecked()) m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorRMS(0) << ";";
    if (m_Controls->m_settingPosErrorMedian->isChecked()) m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorMedian(0) << ";";
    if (m_Controls->m_settingPosErrorMinMax->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorMax(0) << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetPositionErrorMin(0) << ";";
    }

    if (m_Controls->m_settingEulerMean->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetEulerAnglesMean(0)[0] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetEulerAnglesMean(0)[1] << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetEulerAnglesMean(0)[2] << ";";
    }

    if (m_Controls->m_settingEulerRMS->isChecked())
    {
      m_CurrentWriteFile << myEvaluationFilter->GetEulerAnglesRMS(0) << ";";
      m_CurrentWriteFile << myEvaluationFilter->GetEulerAnglesRMSDegree(0) << ";";
    }

    m_CurrentWriteFile << "\n";
  }
}


std::vector<mitk::Quaternion> QmitkIGTTrackingDataEvaluationView::GetMeanOrientationsOfAllData(std::vector<mitk::NavigationDataEvaluationFilter::Pointer> allData, bool useSLERP)
{
  std::vector<mitk::Quaternion> returnValue;

  for (auto dataSet : allData)
  {
    if (useSLERP) returnValue.push_back(GetSLERPAverage(dataSet));
    else returnValue.push_back(dataSet->GetQuaternionMean(0));
  }

  return returnValue;
}


std::vector<mitk::NavigationDataEvaluationFilter::Pointer> QmitkIGTTrackingDataEvaluationView::GetAllDataFromUIList()
{
  std::vector<mitk::NavigationDataEvaluationFilter::Pointer> EvaluationDataCollection;

  //start loop and iterate through all files of list: store the evaluation data
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    //create navigation data player
    mitk::NavigationDataCSVSequentialPlayer::Pointer myPlayer = ConstructNewNavigationDataPlayer();
    myPlayer->SetFiletype(mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV);
    myPlayer->SetFileName(m_FilenameVector.at(i));

    //create evaluation filter
    mitk::NavigationDataEvaluationFilter::Pointer myEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();

    //connect pipeline
    for (unsigned int j = 0; j < myPlayer->GetNumberOfOutputs(); ++j)
      myEvaluationFilter->SetInput(j, myPlayer->GetOutput(j));

    //update pipline until number of samlples is reached
    for (int j = 0; j < m_Controls->m_NumberOfSamples->value(); ++j)
      myEvaluationFilter->Update();

    myEvaluationFilter->SetInput(nullptr);
    myPlayer = nullptr;
    EvaluationDataCollection.push_back(myEvaluationFilter);
  }

  return EvaluationDataCollection;
}

void QmitkIGTTrackingDataEvaluationView::CalculateDifferenceAngles()
{
  //Get all data from UI
  std::vector<mitk::NavigationDataEvaluationFilter::Pointer> EvaluationDataCollection = GetAllDataFromUIList();

  //calculation and writing of output data
  //open output file
  m_CurrentAngleDifferencesWriteFile.open(std::string((m_Controls->m_OutputFilename->text() + ".angledifferences.csv").toUtf8()).c_str(), std::ios::out);
  if (m_CurrentAngleDifferencesWriteFile.bad())
  {
    MessageBox("Error: Can't open output file for angle differences calculation!");
    return;
  }
  //write header
  WriteDifferenceAnglesHeader();
  //compute angle differences
  QString pos1 = "invalid";
  QString pos2 = "invalid";
  //now iterate through all evaluation data and calculate the angles
  for (std::size_t i = 0; i < m_FilenameVector.size(); ++i)
  {
    pos1 = QString::fromStdString(itksys::SystemTools::GetFilenameWithoutLastExtension(m_FilenameVector.at(i)));
    for (std::size_t j = 0; j < m_FilenameVector.size(); ++j)
    {
      pos2 = QString::fromStdString(itksys::SystemTools::GetFilenameWithoutLastExtension(m_FilenameVector.at(j)));

      mitk::Quaternion q1;
      mitk::Quaternion q2;

      if (m_Controls->m_DifferencesSLERP->isChecked())
      {
        //compute slerp average
        q1 = GetSLERPAverage(EvaluationDataCollection.at(i));
        q2 = GetSLERPAverage(EvaluationDataCollection.at(j));
      }
      else
      {
        //compute arithmetic average
        q1 = EvaluationDataCollection.at(i)->GetQuaternionMean(0);
        q2 = EvaluationDataCollection.at(j)->GetQuaternionMean(0);
      }

      itk::Vector<double> rotationVec;
      //adapt for Aurora 5D tools: [0,0,1000]
      rotationVec[0] = 10000; //X
      rotationVec[1] = 0; //Y
      rotationVec[2] = 0; //Z
      double AngleBetweenTwoQuaternions = mitk::StaticIGTHelperFunctions::GetAngleBetweenTwoQuaterions(q1, q2, rotationVec);

      //write data set
      WriteDifferenceAnglesDataSet(pos1.toStdString(), pos2.toStdString(), i, j, AngleBetweenTwoQuaternions);
    }
  }

  //close output file
  m_CurrentAngleDifferencesWriteFile.close();
}

void QmitkIGTTrackingDataEvaluationView::WriteDifferenceAnglesHeader()
{
  m_CurrentAngleDifferencesWriteFile << "Name;Idx1;Idx2;Angle [Degree]\n";
}

void QmitkIGTTrackingDataEvaluationView::WriteDifferenceAnglesDataSet(std::string pos1, std::string pos2, int idx1, int idx2, double angle)
{
  m_CurrentAngleDifferencesWriteFile << "Angle between " << pos1 << " and " << pos2 << ";" << idx1 << ";" << idx2 << ";" << angle << "\n";
  MITK_INFO << "Angle: " << angle;
}

std::vector<mitk::NavigationData::Pointer> QmitkIGTTrackingDataEvaluationView::GetNavigationDatasFromFile(std::string filename)
{
  std::vector<mitk::NavigationData::Pointer> returnValue = std::vector<mitk::NavigationData::Pointer>();
  std::vector<std::string> fileContentLineByLine = GetFileContentLineByLine(filename);
  for (std::size_t i = 1; i < fileContentLineByLine.size(); ++i) //skip header so start at 1
  {
    returnValue.push_back(GetNavigationDataOutOfOneLine(fileContentLineByLine.at(i)));
  }

  return returnValue;
}

std::vector<std::string> QmitkIGTTrackingDataEvaluationView::GetFileContentLineByLine(std::string filename)
{
  std::vector<std::string> readData = std::vector<std::string>();

  //save old locale
  char * oldLocale;
  oldLocale = setlocale(LC_ALL, nullptr);

  //define own locale
  std::locale C("C");
  setlocale(LC_ALL, "C");

  //read file
  std::ifstream file;
  file.open(filename.c_str(), std::ios::in);
  if (file.good())
  {
    //read out file
    file.seekg(0L, std::ios::beg);  // move to begin of file
    while (!file.eof())
    {
      std::string buffer;
      std::getline(file, buffer);    // read out file line by line
      if (buffer.size() > 0) readData.push_back(buffer);
    }
  }

  file.close();

  //switch back to old locale
  setlocale(LC_ALL, oldLocale);

  return readData;
}

mitk::NavigationData::Pointer QmitkIGTTrackingDataEvaluationView::GetNavigationDataOutOfOneLine(std::string line)
{
  mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();

  QString myLine = QString(line.c_str());

  QStringList myLineList = myLine.split(';');

  mitk::Point3D position;
  mitk::Quaternion orientation;

  bool valid = false;
  if (myLineList.at(2).toStdString() == "1") valid = true;

  position[0] = myLineList.at(3).toDouble();
  position[1] = myLineList.at(4).toDouble();
  position[2] = myLineList.at(5).toDouble();

  orientation[0] = myLineList.at(6).toDouble();
  orientation[1] = myLineList.at(7).toDouble();
  orientation[2] = myLineList.at(8).toDouble();
  orientation[3] = myLineList.at(9).toDouble();

  returnValue->SetDataValid(valid);
  returnValue->SetPosition(position);
  returnValue->SetOrientation(orientation);

  return returnValue;
}

mitk::Quaternion QmitkIGTTrackingDataEvaluationView::GetSLERPAverage(mitk::NavigationDataEvaluationFilter::Pointer evaluationFilter)
{
  mitk::Quaternion average;

  //build a vector of quaternions from the evaulation filter (caution always takes the first (0) input of the filter
  std::vector<mitk::Quaternion> quaternions = std::vector<mitk::Quaternion>();
  for (int i = 0; i < evaluationFilter->GetNumberOfAnalysedNavigationData(0); i++)
  {
    mitk::Quaternion currentq = evaluationFilter->GetLoggedOrientation(i, 0);

    quaternions.push_back(currentq);
  }

  //compute the slerp average using the quaternion averaging class
  mitk::QuaternionAveraging::Pointer myAverager = mitk::QuaternionAveraging::New();
  average = myAverager->CalcAverage(quaternions);

  return average;
}

void QmitkIGTTrackingDataEvaluationView::writeToFile(std::string filename, std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> values)
{
  std::fstream currentFile;
  currentFile.open(filename.c_str(), std::ios::out);
  if (currentFile.bad()) { MITK_WARN << "Cannot open file, aborting!"; return; }
  currentFile << "Description" << ";" << "Error[mm]" << "\n";
  for (auto currentError : values)
  {
    currentFile << currentError.description << ";" << currentError.distanceError << "\n";
  }
  currentFile.close();
}

mitk::NavigationDataCSVSequentialPlayer::Pointer QmitkIGTTrackingDataEvaluationView::ConstructNewNavigationDataPlayer()
{
    bool rightHanded = m_Controls->m_RigthHanded->isChecked();
    QString separator =  m_Controls->m_SeparatorSign->text();
    QChar sepaSign = separator.at(0);
    //char separatorSign;
    char separatorSign = sepaSign.toLatin1();
    //std::string separatorSign = m_Controls->m_SeparatorSign->text().toStdString();
    int sampleCount = m_Controls->m_SampleCount->value();
    bool headerRow = m_Controls->m_HeaderRow->isChecked();
    int xPos = m_Controls->m_XPos->value();
    int yPos = m_Controls->m_YPos->value();
    int zPos = m_Controls->m_ZPos->value();
    bool useQuats = m_Controls->m_UseQuats->isChecked();
    int qx = m_Controls->m_Qx->value();
    int qy = m_Controls->m_Qy->value();
    int qz = m_Controls->m_Qz->value();
    int qr = m_Controls->m_Qr->value();
    int azimuth = m_Controls->m_Azimuth->value();
    int elevation = m_Controls->m_Elevation->value();
    int roll = m_Controls->m_Roll->value();
    bool eulersInRad = m_Controls->m_Radiants->isChecked();
    //need to find the biggest column number to determine the  minimal number of columns the .csv file has to have
    int allInts[] = {xPos, yPos, zPos, qx, qy, qr, azimuth, elevation, roll};
    int minNumberOfColumns = (*std::max_element(allInts, allInts+9)+1); //size needs to be +1 because columns start at 0 but size at 1

    mitk::NavigationDataCSVSequentialPlayer::Pointer navDataPlayer = mitk::NavigationDataCSVSequentialPlayer::New();
    navDataPlayer->SetOptions(rightHanded, separatorSign, sampleCount, headerRow, xPos, yPos, zPos, useQuats,
                              qx, qy, qz, qr, azimuth, elevation, roll, eulersInRad, minNumberOfColumns);
    return navDataPlayer;
}
