/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

// these files have to be included at this location, 
// otherwise visual c++ will produce an internal 
// compiler error :(
#ifdef MBI_INTERNAL
 #include <mitkVesselGraphFileReader.h>
 #include <mitkVesselTreeFileReader.h>
 #include <mitkVesselTreeToLookupTableFilter.h>
 #include <mitkVesselTreeLookupTableProperty.h>
#endif

// C-Standard library includes
#include <stdlib.h>

// STL-related includes
#include <vector>
#include <map>

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

// MITK-related includes
#include "mitkSurface.h"
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

#ifdef MBI_INTERNAL
 #ifdef HAVE_IPDICOM
  #include "mitkDICOMFileReader.h"
 #endif // HAVE_IPDICOM 
 #include "mitkBirdDataFromFileReader.h"
 #include "mitkTrackedUSDataFileParser.h"
 #include "mitkUSLookupTableSource.h"
 #include "mitkCylindricToCartesianFilter.h"
 #include "mitkDSRFileReader.h"
 #include "mitkShapeModelFileReader.h"
#endif // MBI_INTERNAL

mitk::DataTreeNodeFactory::DataTreeNodeFactory()
{
  m_Serie = false;
  this->Modified();
}

mitk::DataTreeNodeFactory::~DataTreeNodeFactory()
{}

void mitk::DataTreeNodeFactory::SetImageSerie(bool serie)
{
  m_Serie = serie;
}

void mitk::DataTreeNodeFactory::GenerateData()
{
  // Test if the file exists.
  if( ! itksys::SystemTools::FileExists( m_FileName.c_str() ) )
  {
    std::string message("File does not exist. Filename = ");
    message += m_FileName;
    throw itk::ImageFileReaderException( __FILE__, __LINE__, message.c_str() );
  }

  // Test if the file can be open for reading access.
  std::ifstream readTester( m_FileName.c_str() );
  if( !readTester )
  {
    std::string message("File cannot be read. Filename = ");
    message += m_FileName;
    throw itk::ImageFileReaderException( __FILE__, __LINE__, message.c_str() );
  }
  // end file tests

  // part for DICOM
  if ( this->FileNameEndsWith( ".dcm" ) || this->FileNameEndsWith( ".DCM" ) 
    || this->FileNameEndsWith( ".ima" ) 
    || this->FileNameEndsWith( ".IMA" ) 
    || this->FilePatternEndsWith( ".dcm" ) 
    || this->FilePatternEndsWith( ".DCM" ) 
    || this->FilePatternEndsWith( ".ima" ) 
    || this->FilePatternEndsWith( ".IMA" ) 
    || (itksys::SystemTools::GetFilenameLastExtension(m_FileName) == "" ) )
  {
    if (m_Serie)
    {
      m_FileName = m_FilePrefix;
    }
    this->ReadFileSeriesTypeDCM();
  }
  else if ( this->FileNameEndsWith( ".gdcm" ) || this->FileNameEndsWith( ".GDCM" ) || this->FilePatternEndsWith( ".gdcm" ) || this->FilePatternEndsWith( ".GDCM" ) )
  {
    if (m_Serie)
    {
      m_FileName = m_FilePrefix;
    }
    this->ReadFileSeriesTypeGDCM();
  }
  else
  {
    /***************************************** new datatreenode factory mechanism **********************************************/
    // the mitkBaseDataIOFactory class returns a pointer of a vector of BaseData objects
    // the IO factories of the file readers are registered in the class mitkBaseDataIOFactory
    // for more details see the doxygen documentation (modules IO)
    bool usedNewDTNF = false;

    if (m_Serie)
    {
      m_FileName = m_FilePrefix;
    }

    std::vector<mitk::BaseData::Pointer>* baseDataVector = mitk::BaseDataIOFactory::CreateBaseDataIO( m_FileName, m_FilePrefix, m_FilePattern, mitk::BaseDataIOFactory::ReadMode );

    if(baseDataVector)
      this->ResizeOutputs((unsigned int)baseDataVector->size());

    for(int i=0; i<(int)baseDataVector->size(); i++)
    {
      mitk::BaseData::Pointer baseData = baseDataVector->at(i);

      if( baseData.IsNotNull() )
      {
        usedNewDTNF = true;
        mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();//this->GetOutput();
        node->SetData(baseData);
        this->SetDefaultCommonProperties( node );

        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
        if(image.IsNotNull())
        {
          this->SetDefaultImageProperties(node);
#ifdef MBI_INTERNAL
          if ( this->FileNameEndsWith( ".pic" ) || this->FileNameEndsWith( ".pic.gz" ) || this->FileNameEndsWith( ".seq" ) )
          {
            if ( image->GetNumberOfChannels() > 1 )
              this->SetDefaultUltraSoundProperties( node );
          }
          else if ( this->FileNameEndsWith( ".TUS" ) || this->FileNameEndsWith( ".tus" ) )
          {
            if ( image->GetNumberOfChannels() > 1 )
            {
              this->SetDefaultUltraSoundProperties( node );
              this->SetDefaultTusProperties( i, node );
            }
          }
#endif // MBI_INTERNAL
        }
#ifdef MBI_INTERNAL
        mitk::VesselTreeData::Pointer vesselTree = dynamic_cast<mitk::VesselTreeData*>(node->GetData());
        if(vesselTree.IsNotNull())
          this->SetDefaultVesselTreeProperties(node);
#endif // MBI_INTERNAL

        mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
        if(surface.IsNotNull())
          this->SetDefaultSurfaceProperties(node);

        node->SetVisibility(true);
        this->SetOutput(i, node);
      }
    }
    /***********************************************************************************************************/
    if(!usedNewDTNF)
    { 
      if ( m_FileName != "" )
      {
#ifdef MBI_INTERNAL
        if ( this->FileNameEndsWith( "HPSONOS.DB" ) || this->FileNameEndsWith( "hpsonos.db" ) )
        {
          std::cout << "reading HPSONOS" << endl;
          this->ReadFileTypeHPSONOS();
        }
#ifdef USE_TUS_READER
        else if ( this->FileNameEndsWith( ".TUS" ) || this->FileNameEndsWith( ".tus" ) )
        {
          std::cout << "reading tus" << endl;
          this->ReadFileTypeTUS();
        }
#endif
#ifdef HAVE_IPDICOM
        else if ( this->FileNameEndsWith( ".IPDCM" ) || this->FileNameEndsWith( ".ipdcm" ) )
        {
          this->ReadFileTypeIPDCM();
        }
#endif /* HAVE_IPDICOM */
#endif /* MBI_INTERNAL */
      }
      // part for series of data
      else if ( m_FilePattern != "" && m_FilePrefix != "" )
      {
          this->ReadFileSeriesTypeITKImageSeriesReader();
      }
    }
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
  return m_FileName.find( name ) != std::string::npos;
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

#ifdef MBI_INTERNAL
void mitk::DataTreeNodeFactory::ReadFileTypeHPSONOS()
{
  std::cout << "loading " << m_FileName << " as DSR ... " << std::endl;

  this->ResizeOutputs( 2 );

  mitk::DSRFileReader::Pointer reader;

  reader = mitk::DSRFileReader::New();

  reader->SetFileName( m_FileName.c_str() );

  std::string m_dir = m_FileName.substr(m_FileName.length()-17,6);


  mitk::ImageChannelSelector::Pointer channelSelector = mitk::ImageChannelSelector::New();
  mitk::ImageChannelSelector::Pointer DopplerChannelSelector = mitk::ImageChannelSelector::New();

  channelSelector->SetInput( reader->GetOutput() );
  DopplerChannelSelector->SetInput( reader->GetOutput() );

  reader->UpdateOutputInformation();

  bool haveDoppler = false;
  if ( reader->GetOutput()->IsValidChannel( 0 ) )
  {
    std::cout << "    have channel data 0 (backscatter) ... " << std::endl;
  }

  if ( reader->GetOutput()->IsValidChannel( 1 ) )
  {
    std::cout << "    have channel data 1 (doppler) ... " << std::endl;
    haveDoppler = true;
  }

  mitk::CylindricToCartesianFilter::Pointer cyl2cart = mitk::CylindricToCartesianFilter::New();
  mitk::CylindricToCartesianFilter::Pointer cyl2cartDoppler = mitk::CylindricToCartesianFilter::New();

  cyl2cart->SetTargetXSize( 128 );
  cyl2cartDoppler->SetTargetXSize( 128 );

  //the outside value must be the value interpreted as v=0. In most cases this is 128.
  cyl2cartDoppler->SetOutsideValue( 128 );

  //
  // switch to Backscatter information
  //
  channelSelector->SetChannelNr( 0 );

  mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();

  //
  // insert original (in cylindrical coordinates) Backscatter information
  //
  mitk::DataTreeNode::Pointer node = this->GetOutput( 0 );
  node->SetData( channelSelector->GetOutput() );
  node->SetData( channelSelector->GetOutput() );
  sliceSelector->SetInput( channelSelector->GetInput() );
  mitk::StringProperty::Pointer ultrasoundProp = new mitk::StringProperty( "OriginalBackscatter" );
  node->SetProperty( "ultrasound", ultrasoundProp );

  mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( m_dir+"(OBS)" );
  node->SetProperty( "name", nameProp );

  //node->SetProperty( "layer", new mitk::IntProperty( -11 ) );
  //mitk::LevelWindow levelwindow;
  //levelwindow.SetAuto( sliceSelector->GetOutput() );
  //node->SetLevelWindow( levelwindow, NULL );
  node->SetVisibility( false, NULL );
  //node->SetColor( 1.0, 1.0, 1.0, NULL );
  node->Update();

  SetDefaultImageProperties(node);

  //
  // insert transformed (in cartesian coordinates) Backscatter information
  //
  cyl2cart->SetInput( reader->GetOutput() );
  node = this->GetOutput( 1 );
  node->SetData( reader->GetOutput() );
  node->SetData( cyl2cart->GetOutput() );
  ultrasoundProp = new mitk::StringProperty( "TransformedBackscatter" );
  node->SetProperty( "ultrasound", ultrasoundProp );
  nameProp = new mitk::StringProperty( m_dir+"(TBS)" );
  node->SetProperty( "name", nameProp );
  //node->SetProperty( "layer", new mitk::IntProperty( -10 ) );

  //node->SetLevelWindow( levelwindow, NULL );
  //node->SetColor( 1.0, 1.0, 1.0, NULL );
  node->Update();


  SetDefaultImageProperties(node);

  if ( haveDoppler )
  {
    this->ResizeOutputs( 4 );

    //
    // switch to Doppler information
    //
    DopplerChannelSelector->SetChannelNr( 1 );
    //DopplerChannelSelector->Update();

    //
    // create a Doppler lookup table
    // TODO: HP map must be provided by DSRFilereader, since it
    // may be dependend on data ( baseline shift)
    //
    mitk::USLookupTableSource::Pointer LookupTableSource = mitk::USLookupTableSource::New();
    LookupTableSource->SetUseDSRDopplerLookupTable();
    LookupTableSource->Update();
    mitk::LookupTableSource::OutputTypePointer LookupTable = LookupTableSource->GetOutput();
    mitk::LookupTableProperty::Pointer LookupTableProp = new mitk::LookupTableProperty( LookupTable );

    int start = 128 - 1;
    int end = 128 + 1;
    for (int i=start; i<=end;i++)
      LookupTableProp->GetLookupTable()->ChangeOpacity(i, 0);

    //
    // insert original (in cylindric coordinates) Doppler information
    //
    node = this->GetOutput( 2 );
    node->SetData( DopplerChannelSelector->GetOutput() );
    node->SetData( DopplerChannelSelector->GetOutput() );
    ultrasoundProp = new mitk::StringProperty( "OriginalDoppler" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    nameProp = new mitk::StringProperty( m_dir+"(OD)" );
    node->SetProperty( "name", nameProp );
    //node->SetProperty( "layer", new mitk::IntProperty( -6 ) );

    //mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    //mitk::LevelWindow levelwindow;
    //levelwindow.SetLevelWindow( 128, 255 );
    //levWinProp->SetLevelWindow( levelwindow );
    // set the overwrite LevelWindow
    // if "levelwindow" is used if "levelwindow" is not available
    // else "levelwindow" is used
    // "levelwindow" is not affected by the slider
    //node->GetPropertyList()->SetProperty( "levelWindow", levWinProp );

    node->SetProperty( "LookupTable", LookupTableProp );
    node->SetVisibility( false, NULL );
    node->Update();

    SetDefaultImageProperties(node);

    //
    // insert transformed (in cartesian coordinates) Doppler information
    //
    cyl2cartDoppler->SetInput( DopplerChannelSelector->GetOutput() );

    node = this->GetOutput( 3 );
    node->SetData( cyl2cartDoppler->GetOutput() );
    node->SetData( cyl2cartDoppler->GetOutput() );
    ultrasoundProp = new mitk::StringProperty( "TransformedDoppler" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    nameProp = new mitk::StringProperty( m_dir+"(TD)" );
    node->SetProperty( "name", nameProp );
    //node->SetProperty( "layer", new mitk::IntProperty( -5 ) );

    // set the overwrite LevelWindow
    // if "levelwindow" is used if "levelwindow" is not available
    // else "levelwindow" is used
    // "levelwindow" is not affected by the slider
    //node->GetPropertyList()->SetProperty( "levelWindow", levWinProp );

    node->SetProperty( "LookupTable", LookupTableProp );
    node->Update();


    SetDefaultImageProperties(node);
  }
  std::cout << "...finished!" << std::endl;
}

#ifdef USE_TUS_READER
void mitk::DataTreeNodeFactory::ReadFileTypeTUS()
{
  unsigned int i,count;

    mitk::TrackedUSDataFileParser* parser = new mitk::TrackedUSDataFileParser();
    cout<<m_FileName<<endl;   
    parser->LoadBehavior(m_FileName); 
    
    //Parse die Tus-File and get BirdDataPfadliste+ HpsonoPfadListe
    std::vector< std::string > BirdDataPfadListe = mitk::TrackedUSDataFileParser::GetBirdDataPfadListe();
    
    std::vector< std::string > HpsonoPfadListe = mitk::TrackedUSDataFileParser::GetHpsonoPfadListe();
    std::vector <std::string>::iterator it;

    int OutputNumber=0;
    i=0;// Laufvariable für die internen KanäleBilder aus DSRreader
    count=0;// Laufvariable für die BilderAnzahl
    BirdDataFromFileReader Birdreader;

    for ( it= HpsonoPfadListe.begin( );it != HpsonoPfadListe.end( );it++ )
    {  
      mitk::DataTreeNodeFactory::Pointer HPSonoReader=mitk::DataTreeNodeFactory::New();
      std::string name=*it;

      //Lese USBilder mittels ReadFileTypeHPSONOS()
      HPSonoReader->SetFileName(name.c_str() );           
      HPSonoReader->Update(); 

      OutputNumber+=HPSonoReader->GetNumberOfOutputs(); // Aktualisiere Anzahl Bilder

      this->ResizeOutputs(OutputNumber);

      Birdreader.SetFilename(BirdDataPfadListe[count].c_str()); 
      mitk::BirdData BD= Birdreader.GetBirdData();
      
      //Set PositionProperty
      mitk::Point3D PositionData= BD.GetPositionMean();
      mitk::Point3dProperty::Pointer positionProp;
      positionProp = new mitk::Point3dProperty(PositionData);
      HPSonoReader->GetOutput(0)->SetProperty( "PositionData",positionProp );
      HPSonoReader->GetOutput(1)->SetProperty( "PositionData",positionProp );
      
      Birdreader.getRotationMatrix();
      
      //Set RotationProperty
      mitk::Point4D QuaternionData =BD.GetRotationMean();
      mitk::Point4dProperty::Pointer quaternionProp;
      quaternionProp = new mitk::Point4dProperty(QuaternionData);     
      HPSonoReader->GetOutput(0)->SetProperty( "QuaternionData", quaternionProp );
      HPSonoReader->GetOutput(1)->SetProperty( "QuaternionData", quaternionProp );

      this->SetOutput(i,HPSonoReader->GetOutput(0));
      this->SetOutput(i+1,HPSonoReader->GetOutput(1));

      i+=2;
      count++;
    }
}
#endif //USE_TUS_READER

#ifdef HAVE_IPDICOM
void mitk::DataTreeNodeFactory::ReadFileTypeIPDCM()
{
  std::cout << "loading " << m_FileName << " as DICOM... " << std::endl;

  mitk::DICOMFileReader::Pointer reader = mitk::DICOMFileReader::New();
  reader->SetFileName( m_FileName.c_str() );
  reader->Update();
  mitk::DataTreeNode::Pointer node = this->GetOutput();
  node->SetData( reader->GetOutput() );


  SetDefaultImageProperties(node);


  // add Level-Window property
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( reader->GetOutput() );
  node->SetLevelWindow( levelwindow, NULL );


  std::cout << "...finished!" << std::endl;
}


#endif /* HAVE_IPDICOM */
#endif // MBI_INTERNAL


void mitk::DataTreeNodeFactory::ReadFileSeriesTypeDCM()
{
  std::cout << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as DICOM..." << std::endl;

  typedef itk::Image<short, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef std::vector<std::string> StringContainer;
  typedef itk::DICOMImageIO2 IOType;
  typedef itk::DICOMSeriesFileNames NameGeneratorType;

  std::string dir = this->GetDirectory();
  std::cout << "dir: " << dir << std::endl;

  IOType::Pointer dicomIO = IOType::New();

  // Get the DICOM filenames from the directory
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();
  nameGenerator->SetDirectory( dir.c_str() );

  const StringContainer & seriesUID = nameGenerator->GetSeriesUIDs();
  StringContainer::const_iterator seriesItr = seriesUID.begin();
  StringContainer::const_iterator seriesEnd = seriesUID.end();

  std::cout << "The directory " << dir << "contains the following DICOM Series: " << std::endl;
  while ( seriesItr != seriesEnd )
  {
    std::cout << *seriesItr << std::endl;
    seriesItr++;
  }

  this->ResizeOutputs( seriesUID.size() );

  for ( unsigned int i = 0 ; i < seriesUID.size() ; ++i )
  {
    std::cout << "Reading series " << seriesUID[ i ] << std::endl;
    StringContainer fileNames = nameGenerator->GetFileNames( seriesUID[ i ] );
    StringContainer::const_iterator fnItr = fileNames.begin();
    StringContainer::const_iterator fnEnd = fileNames.end();
    while ( fnItr != fnEnd )
    {
      std::cout << *fnItr << std::endl;
      fnItr++;
    }
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( fileNames );
    reader->SetImageIO( dicomIO );
    try
    {
      reader->Update();

      //Initialize mitk image from itk
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk( reader->GetOutput() );
      image->SetVolume( reader->GetOutput()->GetBufferPointer() );

      //add the mitk image to the node
      mitk::DataTreeNode::Pointer node = this->GetOutput( i );
      node->SetData( image );

      SetDefaultImageProperties(node);

      // set filename without path as string property
      std::string filename = std::string( this->GetBaseFilePrefix() );
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( seriesUID[ i ] );
      node->SetProperty( "name", nameProp );

    }
    catch ( const std::exception & e )
    {
      itkWarningMacro( << e.what() );
      reader->ResetPipeline();
    }
  }
}

void mitk::DataTreeNodeFactory::ReadFileSeriesTypeGDCM()
{
  std::cout << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as DICOM via gdcm..." << std::endl;
  typedef itk::Image<short, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef std::vector<std::string> StringContainer;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames SeriesFileNames;

  ImageIOType::Pointer gdcmIO = ImageIOType::New();
  SeriesFileNames::Pointer it = SeriesFileNames::New();

  // Get the DICOM filenames from the directory
  it->SetInputDirectory( this->GetDirectory() );

  ReaderType::Pointer reader = ReaderType::New();

  const ReaderType::FileNamesContainer & filenames = it->GetInputFileNames();
  unsigned int numberOfFilenames =  filenames.size();
  std::cout << numberOfFilenames << std::endl; 
  for(unsigned int fni = 0; fni<numberOfFilenames; fni++)
  {
    std::cout << "filename # " << fni << " = ";
    std::cout << filenames[fni] << std::endl;
  }
  
  reader->SetFileNames( filenames );
  reader->SetImageIO( gdcmIO );

  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject &excp)
  {
    std::cerr << "Exception thrown while writing the image" << std::endl;
    std::cerr << excp << std::endl;

    return;
  }
    //Initialize mitk image from itk
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( reader->GetOutput() );
    image->SetVolume( reader->GetOutput()->GetBufferPointer() );
    
    //add the mitk image to the node
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( image );
    
    SetDefaultImageProperties(node);
    
    // set filename without path as string property
    std::string filename = std::string( this->GetBaseFilePrefix() );
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( "unknownName.gdcm" );
    node->SetProperty( "name", nameProp );
}




void mitk::DataTreeNodeFactory::ReadFileSeriesTypeITKImageSeriesReader()
{
  typedef itk::Image<int, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef itk::NumericSeriesFileNames NameGenerator;
  typedef std::vector<std::string> StringContainer;
  typedef std::map<unsigned int, std::string> SortedStringContainer;

  StringContainer unmatchedFiles;
  StringContainer matchedFiles;

  //
  // Load Directory
  //
  std::string directory = this->GetDirectory();
  itksys::Directory itkDir;
  if ( !itkDir.Load ( directory.c_str() ) )
  {
    itkWarningMacro ( << "Directory " << directory << " cannot be read!" );
    return ;
  }

  //
  // Get a list of all files in the directory
  //
  for ( unsigned long i = 0; i < itkDir.GetNumberOfFiles(); i++ )
  {
    // Only read files
    std::string filename = directory + "/" + itkDir.GetFile( i );
    if ( itksys::SystemTools::FileIsDirectory( filename.c_str() ) )
      continue;

    // store the filenames without path
    unmatchedFiles.push_back( itkDir.GetFile( i ) );
  }

  //
  // Match the file list against the file prefix and extension,
  // the result should be only the files that should be read
  //
  std::string prefix = this->GetBaseFilePrefix();
  std::string patternExtension = itksys::SystemTools::LowerCase( itksys::SystemTools::GetFilenameLastExtension( m_FilePattern ) );
  for ( StringContainer::iterator it = unmatchedFiles.begin() ; it != unmatchedFiles.end() ; ++it )
  {
    std::string extension = itksys::SystemTools::LowerCase( itksys::SystemTools::GetFilenameLastExtension( *it ) );
    if ( it->find( prefix ) == 0 && extension == patternExtension )
      matchedFiles.push_back( *it );
  }
  if ( matchedFiles.size() == 0 )
  {
    itkWarningMacro( << "Sorry, none of the files matched the prefix!" );
      return ;
  }


  //
  // parse the file names from back to front for digits
  // and convert them to a number. Store the filename and number
  // in a SortedStringContainer
  //
  SortedStringContainer sortedFiles;
  for ( StringContainer::iterator it = matchedFiles.begin() ; it != matchedFiles.end() ; ++it )
  {
    //
    //remove the last extension, until we have a digit at the end, or no extension is left!
    //
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( *it );
    while ( ( baseFilename[ baseFilename.length() - 1 ] < '0' || baseFilename[ baseFilename.length() - 1 ] > '9' ) && itksys::SystemTools::GetFilenameLastExtension( baseFilename ) != "" )
      baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( baseFilename );

    std::string number;
    for ( unsigned int i = baseFilename.length() - 1; ; --i )
    {
      char character = baseFilename[ i ];
      //do we have a digit?
      if ( character >= '0' && character <= '9' )
      {
        number.insert( 0, &character, 1 );
      }
      else
      {
        //end of digit series found, jump out of loop!
        break;
      }
    }
    if ( number.length() == 0 )
    {
      // The file is not numbered, this is an error!
      // Nevertheless, we try the next files.
      itkWarningMacro( << "The filename " << *it << "does not contain a valid digit sequence!" );
    }
    else
    {
      // convert the number string into an integer and
      // insert the filname (including directory) into the SortedStringContainer
      unsigned int num = atoi( number.c_str() );
      sortedFiles.insert( std::make_pair( num, directory + "/" + *it ) );
    }
  }
  if ( sortedFiles.size() == 0 )
  {
    itkWarningMacro( << "Sorry, no numbered files found, I can't load anything..." );
      return ;
  }

  //
  // Convert the sorted string container in a plain sorted vector of strings;
  //
  StringContainer filesToLoad;
  for ( SortedStringContainer::iterator it = sortedFiles.begin() ; it != sortedFiles.end() ; ++it )
  {
    filesToLoad.push_back( it->second );
    itkDebugMacro( << it->second );
  }


  //
  // Finally, initialize the ITK-reader and load the files!
  //
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileNames( filesToLoad );
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

      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( prefix );
      node->SetProperty( "name", nameProp );

      if ( image->GetPixelType().GetNumberOfComponents() == 1 )
      {
        SetDefaultImageProperties(node);
        // add level-window property
        //mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        //mitk::LevelWindow levelwindow;
        //levelwindow.SetAuto( image );
        //levWinProp->SetLevelWindow( levelwindow );
        //node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
      }
    }
  }
  catch ( const std::exception & e )
  {
    itkWarningMacro( << e.what() );
    return ;
  }
}

void mitk::DataTreeNodeFactory::SetDefaultImageProperties(mitk::DataTreeNode::Pointer &node) 
{
  node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
  node->SetProperty( "use color", new mitk::BoolProperty( true ) );
  node->SetProperty( "iilInterpolation", new mitk::BoolProperty( false ) );
  node->SetProperty( "vtkInterpolation", new mitk::BoolProperty( true ) );
  node->SetProperty( "texture interpolation", new mitk::BoolProperty( true ) );
  node->SetProperty( "layer", new mitk::IntProperty(0));
  node->SetProperty( "in plane resample extent by geometry", new mitk::BoolProperty( false ) );
  node->SetOpacity(1.0f);
  node->SetColor(1.0,1.0,1.0);  

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull())
  {
    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( image );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
  }
  
  // add a default rainbow lookup table for color mapping
  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
  vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
  vtkLut->SetHueRange(0.6667, 0.0);
  vtkLut->SetTableRange(0.0, 20.0);
  vtkLut->Build();
  mitk::LookupTableProperty::Pointer mitkLutProp = new mitk::LookupTableProperty();
  mitkLutProp->SetLookupTable(mitkLut);
  node->SetProperty( "LookupTable", mitkLutProp );
} 

void mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(mitk::DataTreeNode::Pointer &node)
{
  node->SetProperty( "lineWidth", new mitk::IntProperty(2) );
  node->SetProperty( "layer", new mitk::IntProperty(0));
  node->SetProperty( "material", new mitk::MaterialProperty( 1.0, 1.0, 1.0, 1.0, node.GetPointer() ) );
  node->SetProperty( "scalar visibility", new mitk::BoolProperty(false) );
  node->SetProperty( "color mode", new mitk::BoolProperty(false) );
  node->SetProperty( "representation", new mitk::VtkRepresentationProperty );
  node->SetProperty( "interpolation", new mitk::VtkInterpolationProperty );
  mitk::Surface::Pointer surface = dynamic_cast<Surface*>(node->GetData());
  if(surface.IsNotNull())
  {
    if((surface->GetVtkPolyData() != 0) && (surface->GetVtkPolyData()->GetPointData() != 0) && (surface->GetVtkPolyData()->GetPointData()->GetScalars() != 0))
    {
      node->SetProperty( "scalar visibility", new mitk::BoolProperty(true) );
      node->SetProperty( "color mode", new mitk::BoolProperty(true) );
      node->SetProperty( "scalar mode", new mitk::VtkScalarModeProperty );
    }
  }
}

void mitk::DataTreeNodeFactory::SetDefaultCommonProperties(mitk::DataTreeNode::Pointer &node)
{
  mitk::StringProperty::Pointer pathProp = new mitk::StringProperty( itksys::SystemTools::GetFilenamePath( m_FileName ) );
  node->SetProperty( StringProperty::PATH, pathProp );
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name").GetPointer());
  if(nameProp.IsNull() || nameProp->GetValue()=="No Name!")
  {
    nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );
  }
}

#ifdef MBI_INTERNAL
void mitk::DataTreeNodeFactory::SetDefaultVesselTreeProperties(mitk::DataTreeNode::Pointer &node)
{
  mitk::VesselTreeData::Pointer vesselTree = dynamic_cast<VesselTreeData*>(node->GetData());
  if(vesselTree.IsNotNull()){
    mitk::VesselTreeToLookupTableFilter::Pointer lutGenerator = mitk::VesselTreeToLookupTableFilter::New();
    lutGenerator->SetInput( vesselTree );
    lutGenerator->Update(); 
    mitk::VesselTreeLookupTableProperty::Pointer lutProp = new mitk::VesselTreeLookupTableProperty( dynamic_cast<mitk::VesselTreeLookupTable*>( lutGenerator->GetOutput() ) );
    node->SetProperty( "VesselTreeLookupTable", lutProp );
  }
}

void mitk::DataTreeNodeFactory::SetDefaultUltraSoundProperties(mitk::DataTreeNode::Pointer &node) 
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull())
  {
    mitk::ImageChannelSelector::Pointer morphologyChannelSelector = mitk::ImageChannelSelector::New();
    mitk::ImageChannelSelector::Pointer dopplerChannelSelector = mitk::ImageChannelSelector::New();

    morphologyChannelSelector->SetInput( image );
    dopplerChannelSelector->SetInput( image );

    // insert morphology
//    mitk::DataTreeNode::Pointer node = this->GetOutput( 0 );
    node->SetData( morphologyChannelSelector->GetOutput() );
    mitk::StringProperty::Pointer ultrasoundProp = new mitk::StringProperty( "TransformedBackscatter" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName()+" (morphology)" );
    node->SetProperty( "name", nameProp );

    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( morphologyChannelSelector->GetOutput() );
    node->SetVisibility( false, NULL );
    node->Update();

    SetDefaultImageProperties(node);

    // now deal with Doppler
    this->ResizeOutputs( 2 );
    dopplerChannelSelector->SetChannelNr( 1 );

    // create a Doppler lookup table
    // TODO: map must depend on velocity meta information ( e.g., baseline shift)
    mitk::USLookupTableSource::Pointer LookupTableSource = mitk::USLookupTableSource::New();
    LookupTableSource->SetUseDSRDopplerLookupTable();
    LookupTableSource->Update();
    mitk::LookupTableSource::OutputTypePointer LookupTable = LookupTableSource->GetOutput();
    mitk::LookupTableProperty::Pointer LookupTableProp = new mitk::LookupTableProperty( LookupTable );

    int start = 128 - 1;
    int end = 128 + 1;
    for (int i=start; i<=end;i++)
      LookupTableProp->GetLookupTable()->ChangeOpacity(i, 0);

    // insert Doppler
    node = this->GetOutput( 1 );
    node->SetData( dopplerChannelSelector->GetOutput() );
    SetDefaultImageProperties(node);

    ultrasoundProp = new mitk::StringProperty( "TransformedDoppler" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    nameProp = new mitk::StringProperty( this->GetBaseFileName()+" (Doppler)" );
    node->SetProperty( "name", nameProp );
    node->SetProperty( "layer", new mitk::IntProperty( -6 ) );

    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    levelwindow.SetLevelWindow( 128, 255 );
    levWinProp->SetLevelWindow( levelwindow );
    // set the overwrite LevelWindow
    // if "levelwindow" is used if "levelwindow" is not available
    // else "levelwindow" is used
    // "levelwindow" is not affected by the slider
    node->GetPropertyList()->SetProperty( "levelWindow", levWinProp );

    levWinProp = new mitk::LevelWindowProperty();
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

    node->SetProperty( "LookupTable", LookupTableProp );
    node->SetVisibility( false, NULL );
    node->Update();
  }
}

void mitk::DataTreeNodeFactory::SetDefaultTusProperties(int count, mitk::DataTreeNode::Pointer &node) 
{
  mitk::TrackedUSDataFileParser* parser = new mitk::TrackedUSDataFileParser();
  parser->LoadBehavior(m_FileName);

  std::vector< std::string > BirdDataPfadListe = mitk::TrackedUSDataFileParser::GetBirdDataPfadListe();
  std::vector< std::string > HpsonoPfadListe = mitk::TrackedUSDataFileParser::GetHpsonoPfadListe();
  std::vector <std::string>::iterator it;

  BirdDataFromFileReader Birdreader;

  Birdreader.SetFilename(BirdDataPfadListe[count].c_str());
  mitk::BirdData BD = Birdreader.GetBirdData();

  mitk::DoublePoint3D pData=BD.GetPositionMean();
  mitk::Point3D PositionData;
  PositionData[0]=pData[0];
  PositionData[1]=pData[1];
  PositionData[2]=pData[2];

  mitk::Point3dProperty::Pointer positionProp;
  positionProp = new mitk::Point3dProperty(PositionData);
  node->SetProperty( "PositionData",positionProp );

  //Set RotationProperty
  mitk::DoublePoint4D QData =BD.GetRotationMean();
  mitk::Point4D QuaternionData;
  QuaternionData[0]=QData[0];
  QuaternionData[1]=QData[1];
  QuaternionData[2]=QData[2];
  QuaternionData[3]=QData[3];

  mitk::Point4dProperty::Pointer quaternionProp;
  quaternionProp = new mitk::Point4dProperty(QuaternionData);    
  node->SetProperty( "QuaternionData", quaternionProp );
}
#endif // MBI_INTERNAL
