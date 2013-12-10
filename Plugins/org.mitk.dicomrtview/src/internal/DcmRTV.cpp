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
#include "DcmRTV.h"

#include <DataStructures/mitkRTConstants.h>
#include <DataStructures/mitkIsoDoseLevelSetProperty.h>
#include <DataStructures/mitkIsoDoseLevelVectorProperty.h>

#include "mitkSegmentationObjectFactory.h"


// Qt
#include <QMessageBox>


const std::string DcmRTV::VIEW_ID = "org.mitk.views.dcmrtv";

DcmRTV::DcmRTV()
{
    mitk::IsoDoseLevel::ColorType color;
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    m_freeIsoValues = mitk::IsoDoseLevelVector::New();
    m_freeIsoValues->push_back(mitk::IsoDoseLevel::New(0.0,color,true,false));

  //    std::string m_VolumeDir = MITK_ROOT;
  //    m_VolumeDir += "../mbi/Plugins/org.mbi.gui.qt.tofoscopy";
  //    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_VolumeDir.c_str(), false );
  //    mitk::ShaderRepository::Pointer shaderRepository = mitk::ShaderRepository::GetGlobalShaderRepository();
      mitk::CoreServicePointer<mitk::IShaderRepository> shadoRepo(mitk::CoreServices::GetShaderRepository());

      std::string path = "/home/riecker/mitkShaderLighting.xml"; //mitk::StandardFileLocations::GetInstance()->FindFile("mitkShaderTOF.xml");
      std::string isoShaderName = "mitkIsoLineShader";
      MITK_INFO << "shader found under: " << path;
      std::ifstream str(path.c_str());
      shadoRepo->LoadShader(str,isoShaderName);
}

DcmRTV::~DcmRTV(){}

void DcmRTV::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void DcmRTV::CreateQtPartControl( QWidget *parent )
{
  RegisterSegmentationObjectFactory();
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
  connect( m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(LoadRTDoseFile()) );
  connect( m_Controls.btn_isolines, SIGNAL(clicked()), this ,SLOT(LoadIsoLines()));

  connect(m_Controls.btnUpdate, SIGNAL(clicked()), this, SLOT(OnUpdateButtonClicked()));
  connect(m_Controls.freeSlider, SIGNAL(valueChanged(int)), m_Controls.freeBox, SLOT(setValue(int)));
  connect(m_Controls.freeBox, SIGNAL(valueChanged(int)), this, SLOT(OnFreeIsoValueChanged(int)));
  connect(m_Controls.spinPrescribedDose, SIGNAL(valueChanged(double)), this, SLOT(OnPrescribedDoseChanged(double)));
}

void DcmRTV::OnPrescribedDoseChanged(double value)
{
  if (m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetFloatProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(), m_Controls.spinPrescribedDose->value());
  }
}

void DcmRTV::LoadIsoLines()
{
//  MITK_INFO << "\a";
  bool result;
  if(m_selectedNode->GetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(),result) && result)
  {
    m_selectedNode->SetProperty("shader",mitk::ShaderProperty::New("mitkIsoLineShader"));
//    m_selectedNode->SetProperty("shader.mitkIsoLineShader.HoleSize", mitk::FloatProperty::New(90.0f));
//    m_selectedNode->SetProperty("shader.mitkIsoLineShader.CustomISO", mitk::FloatProperty::New(m_Controls.freeBox->value()));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    MITK_WARN << "Selected file has to be a Dose file!";
  }
}

//void DcmRTV::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
//                                             const QList<mitk::DataNode::Pointer>& nodes )
//{
//  // iterate all selected objects, adjust warning visibility
//  foreach( mitk::DataNode::Pointer node, nodes )
//  {
//    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
//    {
//      m_Controls.buttonPerformImageProcessing->setEnabled( true );
//      return;
//    }
//  }

//  m_Controls.buttonPerformImageProcessing->setEnabled( true );
//}

void DcmRTV::OnFreeIsoValueChanged(int value)
{
  if(m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetProperty("shader.mitkIsoLineShader.CustomISO", mitk::FloatProperty::New(m_Controls.freeBox->value()));
    (*m_freeIsoValues)[0]->SetDoseValue(value/100.0);
    mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(m_freeIsoValues);
    m_selectedNode->SetProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void DcmRTV::OnUpdateButtonClicked()
{
  if(m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(), true);
    m_selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), true);
    m_selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), true);
    m_selectedNode->SetFloatProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(), m_Controls.spinPrescribedDose->value());

    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

    mitk::IsoDoseLevel::ColorType color;
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.4;
    mitk::IsoDoseLevel::Pointer level = mitk::IsoDoseLevel::New(0.01,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.0;
    color[1] = 0.2;
    color[2] = 0.8;
    level = mitk::IsoDoseLevel::New(0.1,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.0;
    color[1] = 0.4;
    color[2] = 1.0;
    level = mitk::IsoDoseLevel::New(0.2,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.0;
    color[1] = 0.7;
    color[2] = 1.0;
    level = mitk::IsoDoseLevel::New(0.3,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.0;
    color[1] = 0.7;
    color[2] = 0.6;
    level = mitk::IsoDoseLevel::New(0.4,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.0;
    color[1] = 1.0;
    color[2] = 0.3;
    level = mitk::IsoDoseLevel::New(0.5,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 0.6;
    level = mitk::IsoDoseLevel::New(0.6,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 0.0;
    level = mitk::IsoDoseLevel::New(0.7,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.8;
    color[2] = 0.0;
    level = mitk::IsoDoseLevel::New(0.8,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.5;
    color[2] = 0.0;
    level = mitk::IsoDoseLevel::New(0.9,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.4;
    color[2] = 0.0;
    level = mitk::IsoDoseLevel::New(0.95,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.2;
    color[2] = 0.0;
    level = mitk::IsoDoseLevel::New(1.0,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.0;
    color[2] = 0.3;
    level = mitk::IsoDoseLevel::New(1.07,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.0;
    color[2] = 0.4;
    level = mitk::IsoDoseLevel::New(1.1,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.4;
    color[2] = 0.4;
    level = mitk::IsoDoseLevel::New(1.2,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 1.0;
    color[1] = 0.7;
    color[2] = 0.7;
    level = mitk::IsoDoseLevel::New(1.3,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.8;
    color[1] = 0.6;
    color[2] = 0.6;
    level = mitk::IsoDoseLevel::New(1.4,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    color[0] = 0.65;
    color[1] = 0.4;
    color[2] = 0.4;
    level = mitk::IsoDoseLevel::New(1.5,color,true,true);
    levelSet->SetIsoDoseLevel(level);

    mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(levelSet);
    m_selectedNode->SetProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);


    (*m_freeIsoValues)[0]->SetDoseValue(m_Controls.freeBox->value()/100.0);
    mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(m_freeIsoValues);
    m_selectedNode->SetProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void DcmRTV::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes )
{
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
  if (dataNodes.empty())
  {
    m_selectedNode = NULL;
    m_Controls.btn_isolines->setDisabled(true);
  }
  else
  {
    m_selectedNode = dataNodes[0];
    bool result;
    if(m_selectedNode->GetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(),result) && result)
    {
      m_Controls.btn_isolines->setEnabled(m_selectedNode.IsNotNull());
    }
    else
    {
      m_Controls.btn_isolines->setDisabled(true);
    }
  }
  m_Controls.btnUpdate->setEnabled(m_selectedNode.IsNotNull());
//  m_Controls.btn_isolines->setEnabled(m_selectedNode.IsNotNull());
}

void DcmRTV::UpdateIsoLines(int value)
{
  QFileDialog dialog;
  dialog.setNameFilter(tr("Images (*.dcm"));

  mitk::DicomSeriesReader::StringContainer files;
  QStringList fileNames = dialog.getOpenFileNames();
  if(fileNames.empty())
  {
    return;
  }
  QStringListIterator fileNamesIterator(fileNames);
  while(fileNamesIterator.hasNext())
  {
    files.push_back(fileNamesIterator.next().toStdString());
  }

  std::string tmp = files.front();
  const char* filename = tmp.c_str();
  char* ncFilename = const_cast<char*>(filename);

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

  mitk::DataNode::Pointer mitkImage = mitk::DataNode::New();
  mitkImage = _DicomRTReader->LoadRTDose(dataset,ncFilename);

  mitk::Image::Pointer picture = dynamic_cast<mitk::Image*>(mitkImage->GetData());

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  double omg = value;
  polyData = _DicomRTReader->GetIsoLine(omg, picture->GetVtkImageData());
  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(polyData);
  for(int i=0;i<polyData->GetLength();++i)
  {
    double* ar;
    polyData->GetPoint(i,ar);
  }

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(surface);
  GetDataStorage()->Add(node);
}

void DcmRTV::DoImageProcessing()
{
  QFileDialog dialog;
  dialog.setNameFilter(tr("Images (*.dcm"));

  mitk::DicomSeriesReader::StringContainer files;
  QStringList fileNames = dialog.getOpenFileNames();
  if(fileNames.empty())
  {
    return;
  }
  QStringListIterator fileNamesIterator(fileNames);
  while(fileNamesIterator.hasNext())
  {
    files.push_back(fileNamesIterator.next().toStdString());
  }

  std::string tmp = files.front();
  const char* filename = tmp.c_str();
  char* filenameC = const_cast<char*>(filename);

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

  mitk::DicomRTReader::Pointer readerRT = mitk::DicomRTReader::New();
  std::deque<mitk::ContourModelSet::Pointer> modelVector;
  modelVector = readerRT->ReadDicomFile(filenameC);

  if(modelVector.empty())
  {
    QMessageBox::information(NULL, "Error", "Vector is empty ...");
  }

  for(int i=0; i<modelVector.size();i++)
  {
    mitk::DataNode::Pointer x = mitk::DataNode::New();
    x->SetData(modelVector.at(i));
    x->SetProperty("name", modelVector.at(i)->GetProperty("name"));
    x->SetVisibility(true);
    GetDataStorage()->Add(x);
  }
  mitk::TimeSlicedGeometry::Pointer geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
}

void DcmRTV::LoadRTDoseFile()
{
  QFileDialog dialog;
  dialog.setNameFilter(tr("Images (*.dcm"));

  mitk::DicomSeriesReader::StringContainer files;
  QStringList fileNames = dialog.getOpenFileNames();
  if(fileNames.empty())
  {
    return;
  }
  QStringListIterator fileNamesIterator(fileNames);
  while(fileNamesIterator.hasNext())
  {
    files.push_back(fileNamesIterator.next().toStdString());
  }

  std::string tmp = files.front();
  const char* filename = tmp.c_str();
  char* ncFilename = const_cast<char*>(filename);

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

  mitk::DataNode::Pointer mitkImage = mitk::DataNode::New();
  mitkImage = _DicomRTReader->LoadRTDose(dataset,ncFilename);

  GetDataStorage()->Add(mitkImage);

//  mitk::DataNode::Pointer doseNode = GetDataStorage()->GetNamedNode("DicomRT Dose");
//  mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(doseNode->GetData());
//  vtkMarchingSquares* contourFilter = vtkMarchingSquares::New();
//  std::deque<mitk::Surface::Pointer> surfaceStorage;

//    mitk::ExtractImageFilter::Pointer imageExtractor = mitk::ExtractImageFilter::New();
//    mitk::DataNode::Pointer img = this->GetDataStorage()->GetNamedNode("abc");
//    mitk::Image::Pointer picture = dynamic_cast<mitk::Image*>(img->GetData());
//    imageExtractor->SetInput( picture );
//    imageExtractor->SetSliceDimension( 2 );
//    imageExtractor->SetSliceIndex( 12 );
//    imageExtractor->Update();

//    vtkMarchingSquares* contourFilter = vtkMarchingSquares::New();
//    vtkPolyData* polyData = vtkPolyData::New();

//    contourFilter->SetInput(imageExtractor->GetOutput()->GetVtkImageData());
//    contourFilter->SetNumberOfContours(1);
//    contourFilter->SetValue(0,0.5);
//    polyData = contourFilter->GetOutput();

//    mitk::Surface::Pointer c = mitk::Surface::New();
//    c->SetVtkPolyData(polyData);
//    mitk::Geometry3D::Pointer geo = doseImage->GetGeometry()->Clone();
//    mitk::Vector3D spacing;
//    spacing.Fill(1);
//    geo->SetSpacing(spacing);
//    surfaceStorage.push_back(c);

//    mitk::DataNode::Pointer extractNode = mitk::DataNode::New();
//    mitk::Image::Pointer stuff = dynamic_cast<mitk::Image*>(imageExtractor->GetOutput());
//    mitk::Geometry3D::Pointer geo2 = stuff->GetGeometry()->Clone();
//    mitk::Vector3D spacing2;
//    spacing2.Fill(1);
//    geo2->SetSpacing(spacing2);
//    stuff->SetGeometry(geo);
//    extractNode->SetData(stuff);
//    extractNode->SetName("ExtractImage");
//    GetDataStorage()->Add(extractNode);

//    mitk::DataNode::Pointer contourNode = mitk::DataNode::New();
//    c->SetGeometry(geo2);
//    contourNode->SetData(c);
//    contourNode->SetName("IsoDose");
//    GetDataStorage()->Add(contourNode);

  mitk::TimeSlicedGeometry::Pointer geo3 = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo3 );
}
