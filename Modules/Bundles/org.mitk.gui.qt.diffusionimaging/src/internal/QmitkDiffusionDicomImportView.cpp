/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-22 11:00:35 +0200 (Fr, 22 Mai 2009) $
Version:   $Revision: 10185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkDiffusionDicomImportView.h"

// qt includes
#include <QFileDialog>

// itk includes
#include "itkTimeProbesCollectorBase.h"
#include "itkGDCMSeriesFileNames.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"

// diffusion module includes
#include "mitkDicomDiffusionImageHeaderReader.h"
#include "mitkGroupDiffusionHeadersFilter.h"
#include "mitkDicomDiffusionImageReader.h"
#include "mitkDiffusionImage.h"

const std::string QmitkDiffusionDicomImport::VIEW_ID = "org.mitk.views.diffusiondicomimport";


QmitkDiffusionDicomImport::QmitkDiffusionDicomImport(QObject *parent, const char *name)
: QmitkFunctionality(), m_Controls(NULL), m_MultiWidget(NULL)
{
}

QmitkDiffusionDicomImport::~QmitkDiffusionDicomImport()
{}

void QmitkDiffusionDicomImport::CreateQtPartControl(QWidget *parent)
{
  m_Parent = parent;
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkDiffusionDicomImportControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_DicomLoadUseSeriesDetailsCheckbox->setChecked(true);
    m_Controls->m_DicomLoadAcqNumberCheckbox->setChecked(true);
    m_Controls->m_DicomLoadRecursiveCheckbox->setChecked(true);
    m_Controls->m_DicomLoadAverageDuplicatesCheckbox->setChecked(true);

    AdvancedClicked();
    AverageClicked();
  }
}



void QmitkDiffusionDicomImport::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_AddFoldersButton, SIGNAL(clicked()), 
      this, SLOT(DicomLoadAddFolderNames()) );    
    connect( m_Controls->m_DeleteFoldersButton, SIGNAL(clicked()), 
      this, SLOT(DicomLoadDeleteFolderNames()) );
    connect( m_Controls->m_DicomLoadStartLoadButton, SIGNAL(clicked()), 
      this, SLOT(DicomLoadStartLoad()) );
    connect( m_Controls->m_Advanced, SIGNAL(clicked()), 
      this, SLOT(AdvancedClicked()) );    
    connect( m_Controls->m_DicomLoadAverageDuplicatesCheckbox, SIGNAL(clicked()), 
      this, SLOT(AverageClicked()) );
  }
}

void QmitkDiffusionDicomImport::AdvancedClicked()
{
  m_Controls->frame_2->setVisible(m_Controls->m_Advanced->isChecked());
}

void QmitkDiffusionDicomImport::AverageClicked()
{
  m_Controls->m_Blur->setEnabled(m_Controls->m_DicomLoadAverageDuplicatesCheckbox->isChecked());
}

void QmitkDiffusionDicomImport::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkDiffusionDicomImport::DicomLoadDeleteFolderNames() 
{
  m_Controls->listWidget->clear();
}

void QmitkDiffusionDicomImport::DicomLoadAddFolderNames() 
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( m_Parent, QString("Select folders containing DWI data") );
  w->setFileMode( QFileDialog::Directory );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  m_Controls->listWidget->addItems(w->selectedFiles());
}

void QmitkDiffusionDicomImport::DicomLoadStartLoad() 
{
  itk::TimeProbesCollectorBase clock;

  try
  {

    int nrFolders = m_Controls->listWidget->count();
    QString status;
    mitk::DataNode::Pointer node;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(3*nrFolders);

    while(m_Controls->listWidget->count())
    {
    
      // RETREIVE FOLDERNAME
      QString folderName = m_Controls->listWidget->takeItem(0)->text();

      // PARSING DIRECTORY
      clock.Start("Directory Parsing");
      status = QString("Parsing directory %1").arg(folderName);
      mitk::StatusBar::GetInstance()->DisplayText(status.toAscii());
      typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
      typedef InputNamesGeneratorType::Pointer GeneratorPointer;
      GeneratorPointer inputNames = InputNamesGeneratorType::New();
      inputNames->SetUseSeriesDetails(m_Controls->m_DicomLoadUseSeriesDetailsCheckbox->isChecked());
      if(m_Controls->m_DicomLoadAcqNumberCheckbox->isChecked())
        inputNames->AddSeriesRestriction( "0020|0012" );
      inputNames->SetRecursive(m_Controls->m_DicomLoadRecursiveCheckbox->isChecked());
      inputNames->SetInputDirectory( folderName.toAscii() );
      clock.Stop("Directory Parsing");
      mitk::ProgressBar::GetInstance()->Progress();

      // READING HEADER-INFOS
      clock.Start("Header Reading");
      status = QString("Reading Headers %1").arg(folderName);
      mitk::StatusBar::GetInstance()->DisplayText(status.toAscii());
      mitk::DicomDiffusionImageHeaderReader::Pointer headerReader;
      const std::vector<std::string> & seriesUIDs = 
        inputNames->GetSeriesUIDs();
      std::map<std::string, mitk::GroupDiffusionHeadersFilter::InputType> inHeaders;
      unsigned int size = seriesUIDs.size();
      for ( unsigned int i = 0 ; i < size ; ++i )
      {
        std::cout << std::endl << "Image #" << i+1 << "/" << size << " ";
        headerReader = mitk::DicomDiffusionImageHeaderReader::New();
        headerReader->SetSeriesDicomFilenames(
          inputNames->GetFileNames(seriesUIDs[ i ] ));
        headerReader->Update();
        inHeaders[headerReader->GetOutput()->seriesDescription].push_back(headerReader->GetOutput());
        //std::cout << std::endl;
      }
      clock.Stop("Header Reading");
      mitk::ProgressBar::GetInstance()->Progress();

      std::map<std::string,mitk::GroupDiffusionHeadersFilter::InputType>::iterator it;
      for(it = inHeaders.begin(); it!=inHeaders.end(); ++it)
      {

        // GROUP HEADERS
        mitk::GroupDiffusionHeadersFilter::Pointer grouper
          = mitk::GroupDiffusionHeadersFilter::New();
        mitk::GroupDiffusionHeadersFilter::OutputType outHeaders;
        grouper->SetInput((*it).second);
        grouper->Update();
        outHeaders = grouper->GetOutput();

        // READ VOLUMES
        clock.Start("Volume Loading");
        status = QString("Loading Volumes %1").arg(folderName);
        mitk::StatusBar::GetInstance()->DisplayText(status.toAscii());
        typedef short PixelValueType;
        typedef mitk::DicomDiffusionImageReader< PixelValueType, 3 > VolumesReader;
        VolumesReader::Pointer vReader = VolumesReader::New();
        VolumesReader::HeaderContainer hc;

        hc.insert(hc.end(), outHeaders[1].begin(), outHeaders[1].end() );
        hc.insert(hc.end(), outHeaders[2].begin(), outHeaders[2].end() );
        if(hc.size()>1)
        {
          vReader->SetHeaders(hc);
          vReader->Update();
          VolumesReader::OutputImageType::Pointer vecImage;
          vecImage = vReader->GetOutput();
          clock.Stop("Volume Loading");
          
          // CONSTRUCT CONTAINER WITH DIRECTIONS
          typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;
          typedef itk::VectorContainer< unsigned int, 
            GradientDirectionType >                  GradientDirectionContainerType;
          GradientDirectionContainerType::Pointer directions =
            GradientDirectionContainerType::New();
          for(unsigned int i=0; i<hc.size(); i++)
          {
            directions->push_back(hc[i]->DiffusionVector);
          }
          std::cout << std::endl << std::endl;

          // DWI TO DATATREE
          typedef mitk::DiffusionImage<PixelValueType> DiffVolumesType;
          DiffVolumesType::Pointer diffImage = DiffVolumesType::New();
          diffImage->SetDirections(directions);
          diffImage->SetVectorImage(vecImage);
          diffImage->SetB_Value(outHeaders[1][0]->bValue);
          diffImage->InitializeFromVectorImage();
          if(m_Controls->m_DicomLoadAverageDuplicatesCheckbox->isChecked())
            diffImage->AverageRedundantGradients(m_Controls->m_Blur->value());
          //if(m_Controls->m_DicomLoadDuplicateIfSingleSliceCheckbox->isChecked())
          //  diffVolumes->DuplicateIfSingleSlice();
          node=mitk::DataNode::New();
          node->SetData( diffImage );
          GetDefaultDataStorage()->Add(node);
          std::string descr   = (((*it).second)[0])->seriesDescription;
          int number  = (((*it).second)[0])->seriesNumber;
          char cNumber[255];
          sprintf(cNumber,"_%.2d_",number);
          std::string sNumber(cNumber);
          std::string patname = (((*it).second)[0])->patientName;
          std::string nodename = patname+sNumber+descr;
          SetDwiNodeProperties(node, nodename);
        }

      }

      mitk::ProgressBar::GetInstance()->Progress();

    }
 
    clock.Report();
    
    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
      mitk::RenderingManager::GetInstance()->InitializeViews(
        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    status = QString("Finished Loading %1 Volumes").arg(nrFolders);
    mitk::StatusBar::GetInstance()->DisplayText(status.toAscii());
 }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }

}

void QmitkDiffusionDicomImport::SetDwiNodeProperties(mitk::DataNode::Pointer node, std::string name)
{

  node->SetProperty( "IsDWIRawVolume", mitk::BoolProperty::New( true ) );

  // set foldername as string property
  mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( name );
  node->SetProperty( "name", nameProp );
}