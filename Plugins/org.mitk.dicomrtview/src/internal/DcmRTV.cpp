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
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/2/RTSTRUCT.2.16.840.1.113669.2.931128.509887832.20120106104805.776010.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/1/0_RS.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/L_H/RS1.2.826.0.1.3680043.8.176.2013826104523980.670.5041441575.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/W_K/RS1.2.826.0.1.3680043.8.176.2013826103827986.364.7703564406.dcm";
  char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/Patient1_anonym/Pat1-Spezial^01HIT_ BPL _Schaedel _S4-Vsim_RTStructureSetSeries_5-RTSTRUCT-00001-1.2.826.0.1.3680043.2.1143.1983092986672434422852955193772404798.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/Patient19_anonym/Pat19-Spezial^01HIT_ BPL _Schaedel _S4-Vsim_RTStructureSetSeries_5-RTSTRUCT-00001-1.2.826.0.1.3680043.2.1143.998272335983426758812865773853768684.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/patient_1/surfaces.dcm";

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

  mitk::DicomRTReader::Pointer readerRT = mitk::DicomRTReader::New();
  std::deque<mitk::ContourModelSet::Pointer> modelVector;
  modelVector = readerRT->ReadDicomFile(filename);

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
//  char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/W_K/RD1.2.826.0.1.3680043.8.176.2013826103830726.368.5451166161.dcm";

//  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

//  DcmFileFormat file;
//  OFCondition outp = file.loadFile(filename, EXS_Unknown);
//  if(outp.bad())
//  {
//    QMessageBox::information(NULL,"Error","Cant read the file");
//  }
//  DcmDataset *dataset = file.getDataset();

//  mitk::LookupTable::Pointer mitkLUT;
//  mitkLUT = _DicomRTReader->LoadRTDose(dataset);

//  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
//  mitkLutProp->SetLookupTable(mitkLUT);

//  mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
//  renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );

//  mitk::DicomSeriesReader::StringContainer strCont;
//  strCont.push_back(filename);
//  mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( strCont );
//  node->SetProperty("LookupTable", mitkLutProp);
//  node->SetProperty("Image Rendering.Mode", renderingMode);
//  node->SetName("DicomRT Dose");
//  GetDataStorage()->Add(node);

  QFileDialog dialog;
  dialog.setNameFilter(tr("Images (*.dcm"));
  mitk::DicomSeriesReader::StringContainer files;
  QStringList fileNames = dialog.getOpenFileNames();
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
  node->SetProperty("opacity", mitk::FloatProperty::New(0.3));
  node->SetName("DicomRT Dose");
  GetDataStorage()->Add(node);

  mitk::TimeSlicedGeometry::Pointer geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
}
