/*=========================================================================
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
#include "itksys/SystemTools.hxx"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkMemoryUtilities.h"

// diffusion module includes
#include "mitkDicomDiffusionImageHeaderReader.h"
#include "mitkGroupDiffusionHeadersFilter.h"
#include "mitkDicomDiffusionImageReader.h"
#include "mitkDiffusionImage.h"
#include "mitkNrrdDiffusionImageWriter.h"

#include "gdcmDirectory.h"
#include "gdcmScanner.h"
#include "gdcmSorter.h"
#include "gdcmIPPSorter.h"
#include "gdcmAttribute.h"
#include "gdcmVersion.h"

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

    m_Controls->m_DicomLoadRecursiveCheckbox->setChecked(true);
    m_Controls->m_DicomLoadAverageDuplicatesCheckbox->setChecked(false);

    m_Controls->m_DicomLoadRecursiveCheckbox->setVisible(false);

    AverageClicked();
    AdvancedCheckboxClicked();
  }
}



void QmitkDiffusionDicomImport::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_AddFoldersButton, SIGNAL(clicked()), this, SLOT(DicomLoadAddFolderNames()) );
    connect( m_Controls->m_DeleteFoldersButton, SIGNAL(clicked()), this, SLOT(DicomLoadDeleteFolderNames()) );
    connect( m_Controls->m_DicomLoadStartLoadButton, SIGNAL(clicked()), this, SLOT(DicomLoadStartLoad()) );
    connect( m_Controls->m_DicomLoadAverageDuplicatesCheckbox, SIGNAL(clicked()), this, SLOT(AverageClicked()) );
    connect( m_Controls->m_OutputSetButton, SIGNAL(clicked()), this, SLOT(OutputSet()) );
    connect( m_Controls->m_OutputClearButton, SIGNAL(clicked()), this, SLOT(OutputClear()) );
    connect( m_Controls->m_Advanced, SIGNAL(clicked()), this, SLOT(AdvancedCheckboxClicked()) );
    connect( m_Controls->m_Remove, SIGNAL(clicked()), this, SLOT(Remove()) );
  }
}


void QmitkDiffusionDicomImport::Remove()
{
  int i = m_Controls->listWidget->currentRow();
  m_Controls->listWidget->takeItem(i);
}

void QmitkDiffusionDicomImport::AdvancedCheckboxClicked()
{
  bool check = m_Controls->
    m_Advanced->isChecked();

  m_Controls->m_AdvancedFrame->setVisible(check);
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
}

void QmitkDiffusionDicomImport::OutputClear()
{
  m_OutputFolderName = "";
  m_OutputFolderNameSet = false;
  m_Controls->m_OutputLabel->setText("... optional out-folder ...");
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
  mitk::StatusBar::GetInstance()->DisplayText(status.toAscii());
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
  mitk::StatusBar::GetInstance()->DisplayErrorText(status.toAscii());
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

void QmitkDiffusionDicomImport::DicomLoadStartLoad()
{
  itk::TimeProbesCollectorBase clock;
  bool imageSuccessfullySaved = true;

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


    std::string folder = m_Controls->m_OutputLabel->text().toStdString();


    if(berry::Platform::IsWindows())
    {
      folder.append("\\import.log");
    }
    else
    {
      folder.append("/import.log");
    }

    ofstream logfile;
    if(m_OutputFolderNameSet) logfile.open(folder.c_str());

    while(m_Controls->listWidget->count())
    {
      // RETREIVE FOLDERNAME
      QListWidgetItem * item  = m_Controls->listWidget->takeItem(0);
      QString folderName = item->text();




      if(m_OutputFolderNameSet) logfile << "Reading " << folderName.toStdString() << '\n';

      // PARSING DIRECTORY
      PrintMemoryUsage();
      clock.Start(folderName.toAscii());
      std::vector<std::string> seriesUIDs(0);
      std::vector<std::vector<std::string> > seriesFilenames(0);

      Status("== Initial Directory Scan ==");
      if(m_OutputFolderNameSet) logfile << "== Initial Directory Scan ==\n";

      gdcm::Directory d;
      d.Load( folderName.toStdString().c_str(), true ); // recursive !
      const gdcm::Directory::FilenamesType &l1 = d.GetFilenames();
      const unsigned int ntotalfiles = l1.size();
      Status(QString(" ... found %1 different files").arg(ntotalfiles));
      if(m_OutputFolderNameSet)logfile << "...found " << ntotalfiles << " different files\n";

      Status("Scanning Headers");
      if(m_OutputFolderNameSet) logfile << "Scanning Headers\n";

      gdcm::Scanner s;
      const gdcm::Tag t1(0x0020,0x000d); // Study Instance UID
      const gdcm::Tag t2(0x0020,0x000e); // Series Instance UID
      const gdcm::Tag t5(0x0028, 0x0010); // number rows
      const gdcm::Tag t6(0x0028, 0x0011); // number cols
      s.AddTag( t1 );
      s.AddTag( t2 );
      s.AddTag( t5 );
      s.AddTag( t6 );

      bool b = s.Scan( d.GetFilenames() );
      if( !b )
      {
        Error("Scanner failed");
        if(m_OutputFolderNameSet )logfile << "ERROR: scanner failed\n";
        continue;
      }

      // Only get the DICOM files:
      gdcm::Directory::FilenamesType l2 = s.GetKeys();
      const int nfiles = l2.size();
      if(nfiles < 1)
      {
        Error("No DICOM files found");
        if(m_OutputFolderNameSet)logfile << "ERROR: No DICOM files found\n";
        continue;
      }
      Status(QString(" ... successfully scanned %1 headers.").arg(nfiles));
      if(m_OutputFolderNameSet) logfile << "...succesfully scanned " << nfiles << " headers\n";

      Status("Sorting");
      if(m_OutputFolderNameSet) logfile << "Sorting\n";

      const gdcm::Scanner::ValuesType &values1 = s.GetValues(t1);

      int nvalues;
      if(m_Controls->m_DuplicateID->isChecked())
      {
        nvalues = 1;
      }
      else
      {
        nvalues = values1.size();
      }

      if(nvalues>1)
      {
        Error("Multiple sSeries tudies found. Please limit to 1 study per folder");
        if(m_OutputFolderNameSet) logfile << "Multiple series found. Limit to one. If you are convinced this is an error use the merge duplicate study IDs option \n";
        continue;
      }

      const gdcm::Scanner::ValuesType &values5 = s.GetValues(t5);
      const gdcm::Scanner::ValuesType &values6 = s.GetValues(t6);
      if(values5.size()>1 || values6.size()>1)
      {
        Error("Folder contains images of unequal dimensions that cannot be combined in one 3d volume. ABORTING.");
        if(m_OutputFolderNameSet) logfile << "Folder contains images of unequal dimensions that cannot be combined in one 3d volume. ABORTING\n.";
        continue;
      }

      const gdcm::Scanner::ValuesType &values2 = s.GetValues(t2);


      int nSeries;
      if(m_Controls->m_DuplicateID->isChecked())
      {
        nSeries = 1;
      }
      else
      {
        nSeries = values1.size();
      }

      gdcm::Directory::FilenamesType files;
      if(nSeries > 1)
      {
        gdcm::Sorter sorter;
        sorter.SetSortFunction( SortBySeriesUID );
        sorter.StableSort( l2 );
        files = sorter.GetFilenames();
      }
      else
      {
        files = l2;
      }

      unsigned int nTotalAcquis = 0;

      if(nfiles % nSeries != 0)
      {
        Error("Number of files in series not equal, ABORTING");
        if(m_OutputFolderNameSet) logfile << "Number of files in series not equal, Some volumes are probably incomplete. ABORTING \n";
        continue;
      }

      int filesPerSeries = nfiles / nSeries;


      gdcm::Scanner::ValuesType::const_iterator it2 = values2.begin();
      for(int i=0; i<nSeries; i++)
      {

        gdcm::Directory::FilenamesType sub( files.begin() + i*filesPerSeries, files.begin() + (i+1)*filesPerSeries);

        gdcm::Scanner s;
        const gdcm::Tag t3(0x0020,0x0012);  // Acquisition ID
        const gdcm::Tag t4(0x0018,0x0024); // Sequence Name (in case acquisitions are equal for all)
        //        const gdcm::Tag t5(0x20,0x32) );    // Image Position (Patient)
        s.AddTag(t3);
        s.AddTag(t4);
        //        s.AddTag(t5);

        bool b = s.Scan( sub );
        if( !b )
        {
          Error("Scanner failed");
          if(m_OutputFolderNameSet) logfile << "Scanner failed\n";
          continue;
        }

        gdcm::Sorter subsorter;
        gdcm::Scanner::ValuesType::const_iterator it;

        const gdcm::Scanner::ValuesType &values3 = s.GetValues(t3);
        const gdcm::Scanner::ValuesType &values4 = s.GetValues(t4);;
        unsigned int nAcquis = values3.size();

        if(nAcquis != 1)
        {
          subsorter.SetSortFunction( SortByAcquisitionNumber );
          it = values3.begin();
        }
        else
        {
          nAcquis = values4.size();
          subsorter.SetSortFunction( SortBySeqName );
          it = values4.begin();
        }
        nTotalAcquis += nAcquis;
        subsorter.Sort( sub );

        if(filesPerSeries % nAcquis != 0)
        {
          Error("Number of files per acquisition not equal, ABORTING");
          if(m_OutputFolderNameSet) logfile << "Number of files per acquisition not equal, ABORTING \n";
          continue;
        }

        int filesPerAcqu = filesPerSeries / nAcquis;

        gdcm::Directory::FilenamesType subfiles = subsorter.GetFilenames();
        for ( unsigned int j = 0 ; j < nAcquis ; ++j )
        {
          std::string identifier = "serie_" + *it2 + "_acquis_" + *it++;

          gdcm::IPPSorter ippsorter;
          gdcm::Directory::FilenamesType ipplist((j)*filesPerAcqu+subfiles.begin(),(j+1)*filesPerAcqu+subfiles.begin());
          ippsorter.SetComputeZSpacing( true );
          if( !ippsorter.Sort( ipplist ) )
          {
            Error(QString("Failed to sort acquisition %1, ABORTING").arg(identifier.c_str()));
            if(m_OutputFolderNameSet) logfile << "Failed to sort acquisition " << identifier.c_str() << " , Aborting\n";
            continue;
          }
          const std::vector<std::string> & list = ippsorter.GetFilenames();
          seriesFilenames.push_back(list);
          seriesUIDs.push_back(identifier.c_str());
        }
        ++it2;
      }

      if(nfiles % nTotalAcquis != 0)
      {
        Error("Number of files per acquisition differs between series, ABORTING");
        if(m_OutputFolderNameSet) logfile << "Number of files per acquisition differs between series, ABORTING \n";
        continue;
      }

      int slices = nfiles/nTotalAcquis;
      Status(QString("Series is composed of %1 different 3D volumes with %2 slices.").arg(nTotalAcquis).arg(slices));
      if(m_OutputFolderNameSet) logfile << "Series is composed of " << nTotalAcquis << " different 3D volumes with " << slices << " slices\n";

      // READING HEADER-INFOS
      PrintMemoryUsage();
      Status(QString("Reading Headers %1").arg(folderName));
      if(m_OutputFolderNameSet) logfile << "Reading Headers "<< folderName.toStdString() << "\n";

      mitk::DicomDiffusionImageHeaderReader::Pointer headerReader;
      mitk::GroupDiffusionHeadersFilter::InputType inHeaders;
      unsigned int size2 = seriesUIDs.size();
      for ( unsigned int i = 0 ; i < size2 ; ++i )
      {
        Status(QString("Reading header image #%1/%2").arg(i+1).arg(size2));
        headerReader = mitk::DicomDiffusionImageHeaderReader::New();
        headerReader->SetSeriesDicomFilenames(seriesFilenames[i]);
        headerReader->Update();
        inHeaders.push_back(headerReader->GetOutput());
        //Status(std::endl;
      }
      mitk::ProgressBar::GetInstance()->Progress();

      //        // GROUP HEADERS
      //        mitk::GroupDiffusionHeadersFilter::Pointer grouper
      //            = mitk::GroupDiffusionHeadersFilter::New();
      //        mitk::GroupDiffusionHeadersFilter::OutputType outHeaders;
      //        grouper->SetInput(inHeaders);
      //        grouper->Update();
      //        outHeaders = grouper->GetOutput();

      // READ VOLUMES
      PrintMemoryUsage();
      if(m_OutputFolderNameSet) logfile << "Loading volumes\n";
      Status(QString("Loading Volumes %1").arg(folderName));
      typedef short PixelValueType;
      typedef mitk::DicomDiffusionImageReader< PixelValueType, 3 > VolumesReader;
      VolumesReader::Pointer vReader = VolumesReader::New();
      VolumesReader::HeaderContainer hc = inHeaders;

      //        hc.insert(hc.end(), outHeaders[1].begin(), outHeaders[1].end() );
      //        hc.insert(hc.end(), outHeaders[2].begin(), outHeaders[2].end() );
      if(hc.size()>1)
      {
        vReader->SetHeaders(hc);
        vReader->Update();
        VolumesReader::OutputImageType::Pointer vecImage;
        vecImage = vReader->GetOutput();
        Status(QString("Volumes Loaded (%1)").arg(folderName));

        // CONSTRUCT CONTAINER WITH DIRECTIONS
        typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;
        typedef itk::VectorContainer< unsigned int,
        GradientDirectionType >                  GradientDirectionContainerType;
        GradientDirectionContainerType::Pointer directions =
            GradientDirectionContainerType::New();
        std::vector<double> b_vals;
        double maxb = 0;
        for(unsigned int i=0; i<hc.size(); i++)
        {
          double bv = hc[i]->bValue;
          if(maxb<bv)
          {
            maxb = bv;
          }
          b_vals.push_back(bv);
        }

        for(unsigned int i=0; i<hc.size(); i++)
        {
          vnl_vector_fixed<double, 3> vect = hc[i]->DiffusionVector;
          vect.normalize();
          vect *= sqrt(b_vals[i]/maxb);
          directions->push_back(vect);
        }

        // DWI TO DATATREE
        PrintMemoryUsage();
        Status(QString("Initializing Diffusion Image"));
        if(m_OutputFolderNameSet) logfile << "Initializing Diffusion Image\n";
        typedef mitk::DiffusionImage<PixelValueType> DiffVolumesType;
        DiffVolumesType::Pointer diffImage = DiffVolumesType::New();
        diffImage->SetDirections(directions);
        diffImage->SetOriginalDirections(directions);
        if(m_Controls->m_DicomLoadDKFZ->isChecked())
        {
          diffImage->CorrectDKFZBrokenGradientScheme(m_Controls->m_Blur->value());
        }
        diffImage->SetVectorImage(vecImage);
        diffImage->SetB_Value(maxb);
        diffImage->InitializeFromVectorImage();
        Status(QString("Diffusion Image initialized"));
        if(m_OutputFolderNameSet) logfile << "Diffusion Image initialized\n";

        if(m_Controls->m_DicomLoadAverageDuplicatesCheckbox->isChecked())
        {
          PrintMemoryUsage();
          Status(QString("Averaging gradient directions"));
          logfile << "Averaging gradient directions\n";
          diffImage->AverageRedundantGradients(m_Controls->m_Blur->value());
        }

        //if(m_Controls->m_DicomLoadDuplicateIfSingleSliceCheckbox->isChecked())
        //  diffVolumes->DuplicateIfSingleSlice();

        QString descr = QString("%1_%2_%3")
                        .arg(((inHeaders)[0])->seriesDescription.c_str())
                        .arg(((inHeaders)[0])->seriesNumber)
                        .arg(((inHeaders)[0])->patientName.c_str());
        descr = descr.trimmed();
        descr = descr.replace(" ", "_");

        if(!m_OutputFolderNameSet)
        {
          node=mitk::DataNode::New();
          node->SetData( diffImage );
          GetDefaultDataStorage()->Add(node);
          SetDwiNodeProperties(node, descr.toStdString().c_str());
          Status(QString("Image %1 added to datastorage").arg(descr));
        }
        else
        {
          typedef mitk::NrrdDiffusionImageWriter<PixelValueType> WriterType;
          WriterType::Pointer writer = WriterType::New();
          QString fullpath = QString("%1/%2.dwi")
                             .arg(m_OutputFolderName)
                             .arg(descr);
          //std::string pathstring = itksys::SystemTools::ConvertToOutputPath(fullpath.toStdString().c_str());

          writer->SetFileName(fullpath.toStdString());
          writer->SetInput(diffImage);
          try
          {
            writer->Update();
          }
          catch (itk::ExceptionObject &ex)
          {
            imageSuccessfullySaved = false;
            Error(QString("%1\n%2\n%3\n%4\n%5\n%6").arg(ex.GetNameOfClass()).arg(ex.GetFile()).arg(ex.GetLine()).arg(ex.GetLocation()).arg(ex.what()).arg(ex.GetDescription()));
            logfile << QString("%1\n%2\n%3\n%4\n%5\n%6").arg(ex.GetNameOfClass()).arg(ex.GetFile()).arg(ex.GetLine()).arg(ex.GetLocation()).arg(ex.what()).arg(ex.GetDescription()).toStdString() << "\n";

            node=mitk::DataNode::New();
            node->SetData( diffImage );
            GetDefaultDataStorage()->Add(node);
            SetDwiNodeProperties(node, descr.toStdString().c_str());
            Status(QString("Image %1 added to datastorage").arg(descr));
            logfile << "Image " << descr.toStdString() << " added to datastorage\n";
            continue ;
          }
          Status(QString("Image %1 written to disc (%1)").arg(fullpath.toStdString().c_str()));
          logfile << "Image " << fullpath.toStdString() << "\n";
        }
      }
      else
      {
        Status(QString("No diffusion information found (%1)").arg(folderName));
        if(m_OutputFolderNameSet) logfile << "No diffusion information found  "<< folderName.toStdString();
      }

      Status(QString("Finished processing %1 with memory:").arg(folderName));
      if(m_OutputFolderNameSet) logfile << "Finished processing " << folderName.toStdString() << "\n";
      PrintMemoryUsage();
      clock.Stop(folderName.toAscii());
      mitk::ProgressBar::GetInstance()->Progress();
      int lwidget = m_Controls->listWidget->count();
      std::cout << lwidget <<std::endl;

      logfile << "\n";

    }

    logfile.close();

    Status("Timing information");
    clock.Report();

    if(!m_OutputFolderNameSet && node.IsNotNull())
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      if (basedata.IsNotNull())
      {
        mitk::RenderingManager::GetInstance()->InitializeViews(
            basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
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
