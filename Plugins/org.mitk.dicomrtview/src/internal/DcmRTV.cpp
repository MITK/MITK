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

  mitk::LookupTable::Pointer mitkLUT;
  mitkLUT = _DicomRTReader->LoadRTDose(dataset);

  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLUT);

  mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
  renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );

  mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( files );
  node->SetProperty("LookupTable", mitkLutProp);
  node->SetProperty("Image Rendering.Mode", renderingMode);
  node->SetProperty("opacity", mitk::FloatProperty::New(1.0));
  node->SetName("DicomRT Dose");
  GetDataStorage()->Add(node);


  mitk::DataNode::Pointer doseNode = GetDataStorage()->GetNamedNode("DicomRT Dose");
  mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(doseNode->GetData());
//  vtkMarchingSquares* contourFilter = vtkMarchingSquares::New();
  int numberOfIsoLines = 5;
  std::deque<mitk::Surface::Pointer> surfaceStorage;

  for(int i=0; i<numberOfIsoLines;i++)
  {
    vtkContourFilter* contourFilter = vtkContourFilter::New();
    vtkPolyData* polyData = vtkPolyData::New();

    contourFilter->SetInput(doseImage->GetVtkImageData());
  //  contourFilter->SetNumberOfContours(5);
  //  contourFilter->SetValue(0,50000);
    contourFilter->GenerateValues(1,i*100*i,(i+1)*200*(i+1));
    polyData = contourFilter->GetOutput();

    mitk::Surface::Pointer c = mitk::Surface::New();
    c->SetVtkPolyData(polyData);
    mitk::Geometry3D::Pointer geo = doseImage->GetGeometry()->Clone();
  //  geo->ChangeImageGeometryConsideringOriginOffset(false);
    mitk::Vector3D spacing;
    spacing.Fill(1);
    geo->SetSpacing(spacing);
    c->SetGeometry(geo);
    surfaceStorage.push_back(c);
  }

  for(int i=0; i<numberOfIsoLines;i++)
  {
    mitk::DataNode::Pointer contourNode = mitk::DataNode::New();
    contourNode->SetData(surfaceStorage.at(i));
    mitk::Color green;
    green[0] = 0.5;
    green[1] = 0.0;
    green[2] = 1.5;
    contourNode->SetColor(green);
    contourNode->SetName("IsoDose");
    GetDataStorage()->Add(contourNode);
  }

  mitk::TimeSlicedGeometry::Pointer geo2 = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo2 );
}
