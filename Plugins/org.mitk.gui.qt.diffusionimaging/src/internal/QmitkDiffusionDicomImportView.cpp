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

#include "QmitkDiffusionDicomImportView.h"

// qt includes
#include <QFileDialog>

// itk includes
#include "itkTimeProbesCollectorBase.h"
#include "itkGDCMSeriesFileNames.h"
#include "itksys/SystemTools.hxx"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkMemoryUtilities.h"
#include "mitkIOUtil.h"

// diffusion module includes
#include "mitkDicomDiffusionImageHeaderReader.h"
#include "mitkDicomDiffusionImageReader.h"
#include "mitkDiffusionImage.h"

#include "mitkDiffusionDICOMFileReader.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"
#include "mitkSortByImagePositionPatient.h"

#include "gdcmDirectory.h"
#include "gdcmScanner.h"
#include "gdcmSorter.h"
#include "gdcmIPPSorter.h"
#include "gdcmAttribute.h"
#include "gdcmVersion.h"

#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

#include <QMessageBox>

const std::string QmitkDiffusionDicomImport::VIEW_ID = "org.mitk.views.diffusiondicomimport";


QmitkDiffusionDicomImport::QmitkDiffusionDicomImport(QObject* /*parent*/, const char* /*name*/)
  : QmitkFunctionality(), m_Controls(NULL), m_MultiWidget(NULL),
    m_OutputFolderName(""), m_OutputFolderNameSet(false)
{
}

QmitkDiffusionDicomImport::QmitkDiffusionDicomImport(const QmitkDiffusionDicomImport& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
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

    m_Controls->m_DicomLoadRecursiveCheckbox->setChecked(false);
    m_Controls->m_DicomLoadAverageDuplicatesCheckbox->setChecked(false);

    m_Controls->m_DicomLoadRecursiveCheckbox->setVisible(true);
    m_Controls->m_OverrideOptionCheckbox->setVisible(false);
    m_Controls->m_SubdirPrefixLineEdit->setVisible(false);
    m_Controls->m_SetPrefixButton->setVisible(false);
    m_Controls->m_ResetPrefixButton->setVisible(false);

    AverageClicked();
  }
}



void QmitkDiffusionDicomImport::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_AddFoldersButton, SIGNAL(clicked()), this, SLOT(DicomLoadAddFolderNames()) );
    connect( m_Controls->m_DeleteFoldersButton, SIGNAL(clicked()), this, SLOT(DicomLoadDeleteFolderNames()) );
    //connect( m_Controls->m_DicomLoadStartLoadButton, SIGNAL(clicked()), this, SLOT(DicomLoadStartLoad()) );
    connect( m_Controls->m_DicomLoadStartLoadButton, SIGNAL(clicked()), this, SLOT(NewDicomLoadStartLoad()) );
    connect( m_Controls->m_DicomLoadAverageDuplicatesCheckbox, SIGNAL(clicked()), this, SLOT(AverageClicked()) );
    connect( m_Controls->m_OutputSetButton, SIGNAL(clicked()), this, SLOT(OutputSet()) );
    connect( m_Controls->m_OutputClearButton, SIGNAL(clicked()), this, SLOT(OutputClear()) );
    connect( m_Controls->m_Remove, SIGNAL(clicked()), this, SLOT(Remove()) );
    connect( m_Controls->m_SetPrefixButton, SIGNAL(clicked()), this, SLOT(SetPrefixButtonPushed()));
    connect( m_Controls->m_ResetPrefixButton, SIGNAL(clicked()), this, SLOT(ResetPrefixButtonPushed()));
    connect( m_Controls->m_DicomLoadRecursiveCheckbox, SIGNAL(clicked()), this, SLOT(RecursiveSettingsChanged()) );
  }
}

void QmitkDiffusionDicomImport::RecursiveSettingsChanged()
{
  m_Controls->m_SubdirPrefixLineEdit->setVisible( m_Controls->m_DicomLoadRecursiveCheckbox->isChecked() );
  m_Controls->m_SetPrefixButton->setVisible( m_Controls->m_DicomLoadRecursiveCheckbox->isChecked() );
  m_Controls->m_SubdirPrefixLineEdit->clear();
  this->m_Controls->m_SubdirPrefixLineEdit->setEnabled(true);
}

void QmitkDiffusionDicomImport::SetPrefixButtonPushed()
{
  m_Prefix = this->m_Controls->m_SubdirPrefixLineEdit->text().toStdString();
  if( !this->m_Controls->m_ResetPrefixButton->isVisible() )
    this->m_Controls->m_ResetPrefixButton->setVisible(true);

  this->m_Controls->m_SubdirPrefixLineEdit->setEnabled(false);
  this->m_Controls->m_ResetPrefixButton->setEnabled(true);
  this->m_Controls->m_SetPrefixButton->setEnabled(false);
}

void QmitkDiffusionDicomImport::ResetPrefixButtonPushed()
{
  m_Controls->m_SubdirPrefixLineEdit->clear();

  this->m_Controls->m_SubdirPrefixLineEdit->setEnabled(true);

  this->m_Controls->m_ResetPrefixButton->setEnabled(false);
  this->m_Controls->m_SetPrefixButton->setEnabled(true);
}

void QmitkDiffusionDicomImport::Remove()
{
  int i = m_Controls->listWidget->currentRow();
  m_Controls->listWidget->takeItem(i);
}

void QmitkDiffusionDicomImport::OutputSet()
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( m_Parent, QString("Select folders containing DWI data") );
  w->setFileMode( QFileDialog::Directory );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  m_OutputFolderName = w->selectedFiles()[0];
  m_OutputFolderNameSet = true;
  m_Controls->m_OutputLabel->setText(m_OutputFolderName);

  // show file override option checkbox
  m_Controls->m_OverrideOptionCheckbox->setVisible(true);
}

void QmitkDiffusionDicomImport::OutputClear()
{
  m_OutputFolderName = "";
  m_OutputFolderNameSet = false;
  m_Controls->m_OutputLabel->setText("... optional out-folder ...");

  // hide file override option checkbox - no output specified
  m_Controls->m_OverrideOptionCheckbox->setVisible(false);
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

bool SortBySeriesUID(gdcm::DataSet const & ds1, gdcm::DataSet const & ds2 )
{
  gdcm::Attribute<0x0020,0x000e> at1;
  at1.Set( ds1 );
  gdcm::Attribute<0x0020,0x000e> at2;
  at2.Set( ds2 );
  return at1 < at2;
}

bool SortByAcquisitionNumber(gdcm::DataSet const & ds1, gdcm::DataSet const & ds2 )
{
  gdcm::Attribute<0x0020,0x0012> at1;
  at1.Set( ds1 );
  gdcm::Attribute<0x0020,0x0012> at2;
  at2.Set( ds2 );
  return at1 < at2;
}

bool SortBySeqName(gdcm::DataSet const & ds1, gdcm::DataSet const & ds2 )
{
  gdcm::Attribute<0x0018, 0x0024> at1;
  at1.Set( ds1 );
  gdcm::Attribute<0x0018, 0x0024> at2;
  at2.Set( ds2 );

  std::string str1 = at1.GetValue().Trim();
  std::string str2 = at2.GetValue().Trim();
  return std::lexicographical_compare(str1.begin(), str1.end(),
                                      str2.begin(), str2.end() );
}

void QmitkDiffusionDicomImport::Status(QString status)
{
  mitk::StatusBar::GetInstance()->DisplayText(status.toLatin1());
  MITK_INFO << status.toStdString().c_str();
}

void QmitkDiffusionDicomImport::Status(std::string status)
{
  mitk::StatusBar::GetInstance()->DisplayText(status.c_str());
  MITK_INFO << status.c_str();
}

void QmitkDiffusionDicomImport::Status(const char* status)
{
  mitk::StatusBar::GetInstance()->DisplayText(status);
  MITK_INFO << status;
}

void QmitkDiffusionDicomImport::Error(QString status)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(status.toLatin1());
  MITK_ERROR << status.toStdString().c_str();
}

void QmitkDiffusionDicomImport::Error(std::string status)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(status.c_str());
  MITK_ERROR << status.c_str();
}

void QmitkDiffusionDicomImport::Error(const char* status)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(status);
  MITK_ERROR << status;
}

void QmitkDiffusionDicomImport::PrintMemoryUsage()
{
  size_t processSize = mitk::MemoryUtilities::GetProcessMemoryUsage();
  size_t totalSize =  mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  float percentage = ( (float) processSize / (float) totalSize ) * 100.0;
  MITK_INFO << "Current memory usage: " << GetMemoryDescription( processSize, percentage );
}

std::string QmitkDiffusionDicomImport::FormatMemorySize( size_t size )
{
  double val = size;
  std::string descriptor("B");
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "KB";
  }
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "MB";
  }
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "GB";
  }
  std::ostringstream str;
  str << std::fixed << std::setprecision(2) << val << " " << descriptor;
  return str.str();
}

std::string QmitkDiffusionDicomImport::FormatPercentage( double val )
{
  std::ostringstream str;
  str << std::fixed << std::setprecision(2) << val << " " << "%";
  return str.str();
}

std::string QmitkDiffusionDicomImport::GetMemoryDescription( size_t processSize, float percentage )
{
  std::ostringstream str;
  str << FormatMemorySize(processSize) << " (" << FormatPercentage( percentage ) <<")" ;
  return str.str();
}

void QmitkDiffusionDicomImport::NewDicomLoadStartLoad()
{
  itk::TimeProbesCollectorBase clock;
  bool imageSuccessfullySaved = true;

  bool has_prefix = true;

  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

    if ( locale.compare(currLocale)!=0 )
    {
      try
      {
        MITK_INFO << " ** Changing locale from " << setlocale(LC_ALL, NULL) << " to '" << locale << "'";
        setlocale(LC_ALL, locale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not set locale " << locale;
      }
    }

    int nrFolders = m_Controls->listWidget->count();

    if(!nrFolders)
    {
      Error(QString("No input folders were selected. ABORTING."));
      return;
    }

    Status(QString("GDCM %1 used for DICOM parsing and sorting!").arg(gdcm::Version::GetVersion()));

    PrintMemoryUsage();
    QString status;
    mitk::DataNode::Pointer node;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(2*nrFolders);

    gdcm::Directory::FilenamesType complete_list;
    while(m_Controls->listWidget->count())
    {
      // RETREIVE FOLDERNAME
      QListWidgetItem * item  = m_Controls->listWidget->takeItem(0);
      QString folderName = item->text();


      if( this->m_Controls->m_DicomLoadRecursiveCheckbox->isChecked() )
      {

        std::string subdir_prefix = "";
        if( has_prefix )
        {
          subdir_prefix = this->m_Prefix;
        }

        itksys::Directory rootdir;
        rootdir.Load( folderName.toStdString().c_str() );

        for( unsigned int idx=0; idx<rootdir.GetNumberOfFiles(); idx++)
        {
          std::string current_path = rootdir.GetFile(idx);
          std::string directory_path = std::string(rootdir.GetPath()) + std::string("/") + current_path;

          MITK_INFO("dicom.loader.inputrootdir.test") << "ProbePath:   " << current_path;
          MITK_INFO("dicom.loader.inputrootdir.test") << "IsDirectory: " << itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
                                                      << " StartsWith: " << itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() );

          // test for prefix
          if(    itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
                 && itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() )
                 )
          {

            gdcm::Directory d;
            d.Load( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ) , false);

            MITK_INFO("dicom.load.subdir.attempt") << "In directory " << itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() );

            const gdcm::Directory::FilenamesType &l1 = d.GetFilenames();
            const unsigned int ntotalfiles = l1.size();
            Status(QString(" ... found %1 different files").arg(ntotalfiles));

            for( unsigned int i=0; i< ntotalfiles; i++)
            {
              complete_list.push_back( l1.at(i) );
            }


          }

        }
      }
      else
      {
        gdcm::Directory d;
        d.Load( folderName.toStdString().c_str(), this->m_Controls->m_DicomLoadRecursiveCheckbox->isChecked() ); // recursive !
        const gdcm::Directory::FilenamesType &l1 = d.GetFilenames();
        const unsigned int ntotalfiles = l1.size();
        Status(QString(" ... found %1 different files").arg(ntotalfiles));

        for( unsigned int i=0; i< ntotalfiles; i++)
        {
          complete_list.push_back( l1.at(i) );
        }

      }



    }

    {
      mitk::DiffusionDICOMFileReader::Pointer gdcmReader = mitk::DiffusionDICOMFileReader::New();
      mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

      // Use tags as in Qmitk
      // all the things that split by tag in DicomSeriesReader
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0010) ); // Number of Rows
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0011) ); // Number of Columns
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0037) ); // Image Orientation (Patient) // TODO add tolerance parameter (l. 1572 of original code)
      // TODO handle as real vectors! cluster with configurable errors!
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x000e) ); // Series Instance UID
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x0050) ); // Slice Thickness
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0008) ); // Number of Frames
      //tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

      // gdcmReader->AddSortingElement( tagSorter );
      //mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

      mitk::DICOMSortCriterion::ConstPointer sorting =
        mitk::SortByImagePositionPatient::New(  // Image Position (Patient)
          //mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0013), // instance number
             mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0012), // aqcuisition number
                mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0008, 0x0032), // aqcuisition time
                   mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0018, 0x1060), // trigger time
                      mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0008, 0x0018) // SOP instance UID (last resort, not really meaningful but decides clearly)
                    ).GetPointer()
                  ).GetPointer()
                ).GetPointer()
             ).GetPointer()
        // ).GetPointer()
        ).GetPointer();
      tagSorter->SetSortCriterion( sorting );

      // mosaic
      gdcmReader->SetResolveMosaic( this->m_Controls->m_SplitMosaicCheckBox->isChecked() );
      gdcmReader->AddSortingElement( tagSorter );
      gdcmReader->SetInputFiles( complete_list );
      try
      {
        gdcmReader->AnalyzeInputFiles();
      }
      catch( const itk::ExceptionObject &e)
      {
        MITK_ERROR << "Failed to analyze data. " << e.what();
      }
      catch( const std::exception &se)
      {
        MITK_ERROR << "Std Exception " << se.what();
      }

      gdcmReader->LoadImages();

      for( int o = 0; o < gdcmReader->GetNumberOfOutputs(); o++ )
      {
        mitk::Image::Pointer loaded_image = gdcmReader->GetOutput(o).GetMitkImage();
        mitk::DiffusionImage<short>::Pointer d_img = static_cast<mitk::DiffusionImage<short>*>( loaded_image.GetPointer() );

        std::stringstream ss;
        ss << "ImportedData_" << o;

        node = mitk::DataNode::New();
        node->SetData( d_img );
        std::string outname;
        d_img->GetPropertyList()->GetStringProperty("diffusion.dicom.importname", outname );

        node->SetName( outname.c_str() );

        GetDefaultDataStorage()->Add(node);
        //SetDwiNodeProperties(node, ss.str() );
        //Status(QString("Image %1 added to datastorage").arg(descr));
      }



    }

    Status("Timing information");
    clock.Report();

    if(!m_OutputFolderNameSet && node.IsNotNull())
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      if (basedata.IsNotNull())
      {
        mitk::RenderingManager::GetInstance()->InitializeViews(
              basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      }
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    try
    {
      MITK_INFO << " ** Changing locale back from " << setlocale(LC_ALL, NULL) << " to '" << currLocale << "'";
      setlocale(LC_ALL, currLocale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not reset locale " << currLocale;
    }
  }
  catch (itk::ExceptionObject &ex)
  {
    Error(QString("%1\n%2\n%3\n%4\n%5\n%6").arg(ex.GetNameOfClass()).arg(ex.GetFile()).arg(ex.GetLine()).arg(ex.GetLocation()).arg(ex.what()).arg(ex.GetDescription()));
    return ;
  }

  if (!imageSuccessfullySaved)
    QMessageBox::warning(NULL,"WARNING","One or more files could not be saved! The according files where moved to the datastorage.");
  Status(QString("Finished import with memory:"));

  PrintMemoryUsage();
}

void QmitkDiffusionDicomImport::SetDwiNodeProperties(mitk::DataNode::Pointer node, std::string name)
{

  node->SetProperty( "IsDWIRawVolume", mitk::BoolProperty::New( true ) );

  // set foldername as string property
  mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( name );
  node->SetProperty( "name", nameProp );
}
