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

#include "mitkSegmentationObjectFactory.h"


// Qt
#include <QMessageBox>


const std::string DcmRTV::VIEW_ID = "org.mitk.views.dcmrtv";

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
}

void DcmRTV::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.buttonPerformImageProcessing->setEnabled( true );
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
    std::stringstream strstr;
    if(i+1<10)
    {
      strstr << "ContourModel0" << i+1;
    }
    else
    {
      strstr << "ContourModel" << i+1;
    }
    x->SetName(strstr.str());
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

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

//  mitk::LookupTable::Pointer mitkLUT;
//  vtkSmartPointer<vtkColorTransferFunction> transFunc = vtkSmartPointer<vtkColorTransferFunction>::New();
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitkImage = _DicomRTReader->LoadRTDose(dataset);

//  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
//  mitkLutProp->SetLookupTable(mitkLUT);

  mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
  renderingMode->SetValue( mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR);

  mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( files );
  mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
  mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
//  mitkTransFunc->SetColorTransferFunction(transFunc);
//  mitkTransFuncProp->SetValue(mitkTransFunc);
//  node->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
//  node->SetProperty("LookupTable", mitkLutProp);
  node->SetData(mitkImage);
//  node->SetProperty("Image Rendering.Mode", renderingMode);
  node->SetProperty("opacity", mitk::FloatProperty::New(1.0));
  node->SetName("DicomRT Dose");
  GetDataStorage()->Add(node);


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

  mitk::Image::Pointer image = mitk::Image::New();
  mitk::PixelType pt = mitk::MakeScalarPixelType<int>();
  unsigned int dim[] = {2,2,2};

  image->Initialize(pt,3,dim);
  image->SetSpacing(1.0);
  mitk::Point3D m_origin;
  m_origin[0] = 0.0;
  m_origin[1] = 0.0;
  m_origin[2] = 0.0;
  image->SetOrigin(m_origin);

  int *pixel = (int*)image->GetData();
  int size = dim[0]*dim[1]*dim[2];
  for(int i=0;i<size;++i,++pixel)
  {
    *pixel=i*1000;
  }

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetName("TestPixture");
  imageNode->SetData(image);

  GetDataStorage()->Add(imageNode);

  mitk::TimeSlicedGeometry::Pointer geo3 = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo3 );
}
