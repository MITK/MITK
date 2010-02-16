/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkConfig.h>
#include <mitkDataTreeNodeFactory.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkCoreObjectFactory.h>
#include <mitkITKImageImport.h>

// C-Standard library includes
#include <stdlib.h>
#include <locale.h>

// STL-related includes
#include <vector>
#include <map>
#include <istream>
#include <cstdlib>
#include <locale>


// VTK-related includes
#include <vtkSTLReader.h>
#include <vtkOBJReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>

#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkStructuredPointsReader.h>

#include <vtkStructuredPoints.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
#include <vtkXMLImageDataReader.h>
#endif

// ITK-related includes
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkImageFileReader.h>
#include <itkImageIOFactory.h>
#include <itkImageIORegion.h>
#include <itkImageSeriesReader.h>
#include <itkDICOMImageIO2.h>
#include <itkDICOMSeriesFileNames.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkNumericSeriesFileNames.h>
#include <itkCommand.h>

// MITK-related includes
#include "mitkSurface.h"
#include "mitkPointSet.h"
#include "mitkStringProperty.h"
#include "mitkProperties.h"
#include "mitkMaterialProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkImage.h"
#include "mitkLookupTableProperty.h"
#include "mitkLookupTable.h"
#include "mitkImageChannelSelector.h"
#include "mitkImageSliceSelector.h"
#include "mitkCoreObjectFactory.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkProgressBar.h"

bool mitk::DataTreeNodeFactory::m_TextureInterpolationActive = false;    // default value for texture interpolation if nothing is defined in global options (see QmitkMainTemplate.ui.h)

mitk::DataTreeNodeFactory::DataTreeNodeFactory()
: m_UseSeriesDetails(true)
{
  m_Serie = false;
  this->Modified();
  //ensure that a CoreObjectFactory has been instantiated
  mitk::CoreObjectFactory::GetInstance();
}

mitk::DataTreeNodeFactory::~DataTreeNodeFactory()
{}

void mitk::DataTreeNodeFactory::SetImageSerie(bool serie)
{
  m_Serie = serie;
}

void mitk::DataTreeNodeFactory::GenerateData()
{
  // IF filename is something.pic, and something.pic does not exist, try to read something.pic.gz
  // if there are both, something.pic and something.pic.gz, only the requested file is read
  // not only for images, but for all formats
  std::ifstream exists(m_FileName.c_str());
  if (!exists)
  {
    std::string testfilename = m_FileName + ".gz";

    std::ifstream exists(testfilename.c_str());
    if (exists.good()) 
    {
      m_FileName += ".gz";
    }
    else
    {
      testfilename = m_FileName + ".GZ";
      std::ifstream exists(testfilename.c_str());
      if (exists.good()) 
      {
        m_FileName += ".GZ";
      }
      else
      {
        std::string message("File does not exist, or cannot be read. Filename = ");
        message += m_FileName;
        MITK_ERROR << message;
        itkExceptionMacro( << message );
      }
    }
  }

  // part for DICOM
  const char *numbers = "0123456789.";
  std::string::size_type first_non_number;
  first_non_number = itksys::SystemTools::GetFilenameName(m_FileName).find_first_not_of ( numbers );

  if ( this->FileNameEndsWith( ".dcm" ) || this->FileNameEndsWith( ".DCM" ) 
    || this->FileNameEndsWith( ".ima" ) 
    || this->FileNameEndsWith( ".IMA" ) 
    || this->FilePatternEndsWith( ".dcm" ) 
    || this->FilePatternEndsWith( ".DCM" ) 
    || this->FilePatternEndsWith( ".gdcm" ) 
    || this->FilePatternEndsWith( ".GDCM" ) 
    || this->FilePatternEndsWith( ".ima" ) 
    || this->FilePatternEndsWith( ".IMA" ) 
    || (itksys::SystemTools::GetFilenameLastExtension(m_FileName) == "" ) 
    || first_non_number == std::string::npos )
  {
    this->ReadFileSeriesTypeDCM();
  }
  else
  {
    bool usedNewDTNF = false;

    // the mitkBaseDataIO class returns a pointer of a vector of BaseData objects
    std::vector<mitk::BaseData::Pointer> baseDataVector = mitk::BaseDataIO::LoadBaseDataFromFile( m_FileName, m_FilePrefix, m_FilePattern, m_Serie );

    if( !baseDataVector.empty() )
      this->ResizeOutputs((unsigned int)baseDataVector.size());

    for(int i=0; i<(int)baseDataVector.size(); i++)
    {
      mitk::BaseData::Pointer baseData = baseDataVector.at(i);

      if( baseData.IsNotNull() )
      {
        usedNewDTNF = true;
        mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
        node->SetData(baseData);
        this->SetDefaultCommonProperties( node );

        this->SetOutput(i, node);
      }
    }
    if(!usedNewDTNF && !( m_FileName != "" ) && !(m_Serie == false))
        ReadFileSeriesTypeITKImageSeriesReader();
  }
}

void mitk::DataTreeNodeFactory::ResizeOutputs( const unsigned int& num )
{
  unsigned int prevNum = this->GetNumberOfOutputs();
  this->SetNumberOfOutputs( num );
  for ( unsigned int i = prevNum; i < num; ++i )
  {
    this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
  }
}

bool mitk::DataTreeNodeFactory::FileNameEndsWith( const std::string& name )
{
  if (m_FileName.size() < name.size()) return false;
  
  return m_FileName.substr(m_FileName.size() - name.size()) == name;
}

bool mitk::DataTreeNodeFactory::FilePatternEndsWith( const std::string& name )
{
  return m_FilePattern.find( name ) != std::string::npos;
}

std::string mitk::DataTreeNodeFactory::GetBaseFileName()
{
  return itksys::SystemTools::GetFilenameName( m_FileName );
}

std::string mitk::DataTreeNodeFactory::GetBaseFilePrefix()
{
  return itksys::SystemTools::GetFilenameName( m_FilePrefix );
}

std::string mitk::DataTreeNodeFactory::GetDirectory()
{
  if ( m_FileName != "" )
    return itksys::SystemTools::GetFilenamePath( m_FileName );
  else if ( m_FilePrefix != "" )
    return itksys::SystemTools::GetFilenamePath( m_FilePrefix );
  else
    return std::string( "" );
}


// a progress callback
void mitk::DataTreeNodeFactory::OnITKProgressEvent(itk::Object *source, const itk::EventObject &) {
  //// Get the value of the progress
  itk::ProcessObject* _ProcessObject = dynamic_cast<itk::ProcessObject*>(source);
  if(!_ProcessObject)
    return;

  float progress = _ProcessObject->GetProgress();
  unsigned int iProgress = static_cast<unsigned int>(progress * 100);

  mitk::ProgressBar::GetInstance()->Progress(iProgress);

  //// Update the progress bar and value in the MITK progress bar
  //m_mitkProgressMeter->value(100 * progress);
  //m_mitkOutProgressCounter->value(100 * progress);

  //// Show or hide progress bar if necessary
  //if(progress < 1.0f && !m_mitkProgressBar->visible())
  //{
  //  m_mitkProgressBar->show();
  //}
  //else if (progress == 1.0f && m_mitkProgressBar->visible())
  //{
  //  m_mitkProgressBar->hide();
  //}

}

void mitk::DataTreeNodeFactory::ReadFileSeriesTypeDCM()
{
  MITK_INFO << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as DICOM..." << std::endl;
    
  const char* previousCLocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  std::locale previousCppLocale( std::cin.getloc() );
  std::locale l( "C" );
  std::cin.imbue(l);

  typedef itk::Image<short, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef std::vector<std::string> StringContainer;
  typedef itk::GDCMImageIO IOType;
  typedef itk::GDCMSeriesFileNames NameGeneratorType;

  std::string dir = this->GetDirectory();

  IOType::Pointer dicomIO = IOType::New();

  // Get the DICOM filenames from the directory
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  // add more criteria to distinguish between different series
  nameGenerator->SetUseSeriesDetails( m_UseSeriesDetails );

  nameGenerator->SetDirectory( dir.c_str() );
 
  // add series restrictions
  std::vector<std::string>::const_iterator it;
  for(it=m_SeriesRestrictions.begin(); it!=m_SeriesRestrictions.end(); it++)
  {
    nameGenerator->AddSeriesRestriction( *it );
  }

  const StringContainer & seriesUIDs = nameGenerator->GetSeriesUIDs();
  StringContainer::const_iterator seriesItr = seriesUIDs.begin();
  StringContainer::const_iterator seriesEnd = seriesUIDs.end();

  MITK_INFO << "The directory " << dir << " contains the following DICOM Series: " << std::endl;
  while ( seriesItr != seriesEnd )
  {
    MITK_INFO << *seriesItr << std::endl;
    seriesItr++;
  }

  unsigned int size = seriesUIDs.size();
  this->ResizeOutputs( size );

  for ( unsigned int i = 0 ; i < size ; ++i )
  {
    MITK_INFO << "Reading series #" << i << ": " << seriesUIDs[ i ] << std::endl;
    StringContainer fileNames = nameGenerator->GetFileNames( seriesUIDs[ i ] );
    StringContainer::const_iterator fnItr = fileNames.begin();
    StringContainer::const_iterator fnEnd = fileNames.end();
    while ( fnItr != fnEnd )
    {
      MITK_INFO << *fnItr << std::endl;
      ++fnItr;
    }

    // a progress command
    itk::MemberCommand<Self>::Pointer  progressCommand = itk::MemberCommand<Self>::New(); 
    progressCommand->SetCallbackFunction( this, &Self::OnITKProgressEvent );

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( fileNames );
    reader->SetImageIO( dicomIO );
    reader->AddObserver(itk::ProgressEvent(),progressCommand);

    try
    {
      mitk::ProgressBar::GetInstance()->AddStepsToDo(100);
      reader->Update();

      if(reader->GetOutput() == NULL)
      {
        MITK_INFO << "no image returned by reader for series #" << i << std::endl;
        continue;
      }

      //Initialize mitk image from itk
      Image::Pointer image = mitk::Image::New();
      image = ImportItkImage(reader->GetOutput());
      image->DisconnectPipeline();

      //add the mitk image to the node
      mitk::DataTreeNode::Pointer node = this->GetOutput( i );
      node->SetData( image );

      //SetDefaultImageProperties(node);

      // set filename without path as string property
      std::string filename = std::string( this->GetBaseFilePrefix() );
      mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( seriesUIDs[ i ] );
      node->SetProperty( "name", nameProp );

    }
    catch ( const std::exception & e )
    {
      itkWarningMacro( << e.what() );
      MITK_ERROR << "skipping series #" << i << " due to exception" << std::endl;
      reader->ResetPipeline();
    }
  }

  setlocale(LC_NUMERIC, previousCLocale);
  std::cin.imbue(previousCppLocale);
}



void mitk::DataTreeNodeFactory::ReadFileSeriesTypeITKImageSeriesReader()
{
  typedef itk::Image<int, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef itk::NumericSeriesFileNames NameGenerator;

  if ( ! this->GenerateFileList() )
  {
    itkWarningMacro( "Sorry, file list could not be generated!" );
    return ;
  }
  if ( m_MatchedFileNames.size() == 0 )
  {
    itkWarningMacro( "Sorry, no files matched the given filename ("<< m_FileName <<")!" );
    return ;
  }

  //
  // Finally, initialize the ITK-reader and load the files!
  //
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileNames( m_MatchedFileNames );
  try
  {
    reader->Update();
    ResizeOutputs( reader->GetNumberOfOutputs() );
    for ( unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i )
    {
      //Initialize mitk image from itk
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk( reader->GetOutput( i ) );
      image->SetVolume( reader->GetOutput( i )->GetBufferPointer() );

      //add the mitk image to the node
      mitk::DataTreeNode::Pointer node = this->GetOutput( i );
      node->SetData( image );

      mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( m_FileName );
      node->SetProperty( "name", nameProp );
    }
  }
  catch ( const std::exception & e )
  {
    itkWarningMacro( << e.what() );
    return ;
  }
}



mitk::ColorProperty::Pointer mitk::DataTreeNodeFactory::DefaultColorForOrgan( const std::string& organ )
{
  static bool initialized = false;
  static std::map< std::string, std::string > s_ColorMap;

  if (!initialized)
  {
    // all lowercase here, please!

    s_ColorMap.insert( std::make_pair( "ankle",               "0xe38686") );
    s_ColorMap.insert( std::make_pair( "appendix",            "0xe38686") );
    s_ColorMap.insert( std::make_pair( "blood vessels",       "0xff3131") );
    s_ColorMap.insert( std::make_pair( "bronchial tree",      "0x3168ff") );
    s_ColorMap.insert( std::make_pair( "bone",                "0xd5d5d5") );
    s_ColorMap.insert( std::make_pair( "brain",               "0xff9cca") );
    s_ColorMap.insert( std::make_pair( "coccyx",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "colon",               "0xe38686") );
    s_ColorMap.insert( std::make_pair( "cyst",                "0xe38686") );
    s_ColorMap.insert( std::make_pair( "elbow",               "0xe38686") );
    s_ColorMap.insert( std::make_pair( "eye",                 "0xe38686") );
    s_ColorMap.insert( std::make_pair( "fallopian tube",      "0xe38686") );
    s_ColorMap.insert( std::make_pair( "fat",                 "0xff2bee") );
    s_ColorMap.insert( std::make_pair( "hand",                "0xe38686") );
    s_ColorMap.insert( std::make_pair( "gall bladder",        "0x567f18") );
    s_ColorMap.insert( std::make_pair( "heart",               "0xeb1d32") );
    s_ColorMap.insert( std::make_pair( "hip",                 "0xe38686") );
    s_ColorMap.insert( std::make_pair( "kidney",              "0xd33f00") );
    s_ColorMap.insert( std::make_pair( "knee",                "0xe38686") );
    s_ColorMap.insert( std::make_pair( "larynx",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "liver",               "0xffcc3d") );
    s_ColorMap.insert( std::make_pair( "lung",                "0x6bdcff") );
    s_ColorMap.insert( std::make_pair( "lymph node",          "0xff0000") );
    s_ColorMap.insert( std::make_pair( "muscle",              "0xff456a") );
    s_ColorMap.insert( std::make_pair( "nerve",               "0xffea4f") );
    s_ColorMap.insert( std::make_pair( "nose",                "0xe38686") );
    s_ColorMap.insert( std::make_pair( "oesophagus",          "0xe38686") );
    s_ColorMap.insert( std::make_pair( "ovaries",             "0xe38686") );
    s_ColorMap.insert( std::make_pair( "pancreas",            "0xf9ab3d") );
    s_ColorMap.insert( std::make_pair( "pelvis",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "penis",               "0xe38686") );
    s_ColorMap.insert( std::make_pair( "pharynx",             "0xe38686") );
    s_ColorMap.insert( std::make_pair( "prostate",            "0xe38686") );
    s_ColorMap.insert( std::make_pair( "rectum",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "sacrum",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "seminal vesicle",     "0xe38686") );
    s_ColorMap.insert( std::make_pair( "shoulder",            "0xe38686") );
    s_ColorMap.insert( std::make_pair( "spinal cord",         "0xf5f93d") );
    s_ColorMap.insert( std::make_pair( "spleen",              "0xf96c3d") );
    s_ColorMap.insert( std::make_pair( "stomach",             "0xf96c3d") );
    s_ColorMap.insert( std::make_pair( "teeth",               "0xfffcd8") );
    s_ColorMap.insert( std::make_pair( "testicles",           "0xe38686") );
    s_ColorMap.insert( std::make_pair( "thyroid",             "0xfff694") );
    s_ColorMap.insert( std::make_pair( "tongue",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "tumor",               "0x937011") );
    s_ColorMap.insert( std::make_pair( "urethra",             "0xf8ff32") );
    s_ColorMap.insert( std::make_pair( "urinary bladder",     "0xf8ff32") );
    s_ColorMap.insert( std::make_pair( "uterus",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "vagina",              "0xe38686") );
    s_ColorMap.insert( std::make_pair( "vertebra",            "0xe38686") );
    s_ColorMap.insert( std::make_pair( "wrist",               "0xe38686") );
    initialized = true;
  }

  std::string lowercaseOrgan(organ);
  for(unsigned int i = 0; i < organ.length(); i++)
  {
    lowercaseOrgan[i] = tolower(lowercaseOrgan[i]);
  }

  std::map< std::string, std::string >::iterator iter = s_ColorMap.find( lowercaseOrgan );
  if ( iter != s_ColorMap.end() )
  {
    std::string hexColor = iter->second;
    std::string hexRed   = std::string("0x") + hexColor.substr( 2, 2 );
    std::string hexGreen = std::string("0x") + hexColor.substr( 4, 2 );
    std::string hexBlue  = std::string("0x") + hexColor.substr( 6, 2 );

    long int red   = strtol( hexRed.c_str(), NULL, 16 );
    long int green = strtol( hexGreen.c_str(), NULL, 16 );
    long int blue  = strtol( hexBlue.c_str(), NULL, 16 );

    return ColorProperty::New( (float)red/ 255.0, (float)green/ 255.0, (float)blue/ 255.0 );
  }
  else
  {
    // a default color (green)
    return ColorProperty::New( 0.0, 1.0, 0.0 );
  }

}

void mitk::DataTreeNodeFactory::SetDefaultCommonProperties(mitk::DataTreeNode::Pointer &node)
{
  // path
  mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenamePath( m_FileName ) );
  node->SetProperty( StringProperty::PATH, pathProp );


  // name already defined?
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
  if(nameProp.IsNull() || (strcmp(nameProp->GetValue(),"No Name!")==0))
  {
    // name already defined in BaseData
    mitk::StringProperty::Pointer baseDataNameProp = dynamic_cast<mitk::StringProperty*>(node->GetData()->GetProperty("name").GetPointer() );
    if(baseDataNameProp.IsNull() || (strcmp(baseDataNameProp->GetValue(),"No Name!")==0))
    {
      // name neither defined in node, nor in BaseData -> name = filename
      if (FileNameEndsWith( ".gz" ))
        m_FileName = m_FileName.substr( 0, m_FileName.length()-3 );

      nameProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenameWithoutLastExtension( m_FileName ) );

      node->SetProperty( "name", nameProp );
    }
    else
    {
      // name defined in BaseData!
      nameProp = mitk::StringProperty::New( baseDataNameProp->GetValue() );
      node->SetProperty( "name", nameProp );
    }
  }
  
  // visibility
  if(!node->GetProperty("visible"))
    node->SetVisibility(true);
}

