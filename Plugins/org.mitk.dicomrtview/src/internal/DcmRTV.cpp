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
}

void DcmRTV::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonPerformImageProcessing->setEnabled( true );
}


void DcmRTV::DoImageProcessing()
{
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/2/RTSTRUCT.2.16.840.1.113669.2.931128.509887832.20120106104805.776010.dcm";
  char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/1/0_RS.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/L_H/RS1.2.826.0.1.3680043.8.176.2013826104523980.670.5041441575.dcm";
  //char* filename="/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/W_K/RS1.2.826.0.1.3680043.8.176.2013826103827986.364.7703564406.dcm";

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  DcmFileFormat file;
  OFCondition outp = file.loadFile(filename, EXS_Unknown);
  if(outp.bad())
  {
    QMessageBox::information(NULL,"Error","Cant read the file");
  }
  DcmDataset *dataset = file.getDataset();

  ContourModelVector result = _DicomRTReader->ReadStructureSet(dataset);

  mitk::DicomRTReader::Pointer readerRT = mitk::DicomRTReader::New();
  ContourModelVector modelVector = readerRT->ReadDicomFile(filename);

  if(result.empty())
  {
    QMessageBox::information(NULL, "Error", "Vector is empty ...");
  }

  for(int i=0;i<modelVector.size();i++)
  {
    mitk::DataNode::Pointer x = mitk::DataNode::New();
    x->SetData(modelVector.at(i));
    x->SetName("ContourModel");
    x->SetVisibility(true);
    GetDataStorage()->Add(x);
  }
}
