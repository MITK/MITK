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

// C-Standard library includes
#include <stdlib.h>

// STL-related includes
#include <vector>
#include <map>
#include <istream>
#include <cstdlib>


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
#include "mitkGridRepresentationProperty.h"
#include "mitkGridVolumeMapperProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"

bool mitk::DataTreeNodeFactory::m_TextureInterpolationActive = true;		// default value for texture interpolation if nothing is defined in global options (see QmitkMainTemplate.ui.h)

mitk::DataTreeNodeFactory::DataTreeNodeFactory()
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
        mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();//this->GetOutput();
        node->SetData(baseData);
        
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
        if(image.IsNotNull())
          SetDefaultImageProperties(node);
       
        mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(node);
        mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
        if(surface.IsNotNull())
          this->SetDefaultSurfaceProperties(node);

        //beware! mitkCoreObjectFactory opens an *.mps file as a mesh, not as a pointset! Thus mitkMestVtkMapper3D is used to map the data
        mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(node->GetData());
        if(pointset.IsNotNull())
          this->SetDefaultPointSetProperties(node);

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

void mitk::DataTreeNodeFactory::ReadFileSeriesTypeDCM()
{
  std::cout << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as DICOM..." << std::endl;

  typedef itk::Image<short, 3> ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typedef std::vector<std::string> StringContainer;
  typedef itk::GDCMImageIO IOType;
  typedef itk::GDCMSeriesFileNames NameGeneratorType;

  std::string dir = this->GetDirectory();
  std::cout << "dir: " << dir << std::endl;

  IOType::Pointer dicomIO = IOType::New();

  // Get the DICOM filenames from the directory
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();
  nameGenerator->SetDirectory( dir.c_str() );

  // add more criteria to distinguish between different series
  nameGenerator->SetUseSeriesDetails( m_UseSeriesDetails );

  // add series restrictions
  std::vector<std::string>::const_iterator it;
  for(it=m_SeriesRestrictions.begin(); it!=m_SeriesRestrictions.end(); it++)
  {
    nameGenerator->AddSeriesRestriction( *it );
  }

  const StringContainer & seriesUID = nameGenerator->GetSeriesUIDs();
  StringContainer::const_iterator seriesItr = seriesUID.begin();
  StringContainer::const_iterator seriesEnd = seriesUID.end();

  std::cout << "The directory " << dir << " contains the following DICOM Series: " << std::endl;
  while ( seriesItr != seriesEnd )
  {
    std::cout << *seriesItr << std::endl;
    seriesItr++;
  }

  this->ResizeOutputs( seriesUID.size() );

  unsigned int size = seriesUID.size();
  for ( unsigned int i = 0 ; i < size ; ++i )
  {
    std::cout << "Reading series #" << i << ": " << seriesUID[ i ] << std::endl;
    StringContainer fileNames = nameGenerator->GetFileNames( seriesUID[ i ] );
    StringContainer::const_iterator fnItr = fileNames.begin();
    StringContainer::const_iterator fnEnd = fileNames.end();
    while ( fnItr != fnEnd )
    {
      std::cout << *fnItr << std::endl;
      ++fnItr;
    }
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( fileNames );
    reader->SetImageIO( dicomIO );
    try
    {
      reader->Update();

      if(reader->GetOutput() == NULL)
      {
        std::cout << "no image returned by reader for series #" << i << std::endl;
        continue;
      }

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
      mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( seriesUID[ i ] );
      node->SetProperty( "name", nameProp );

    }
    catch ( const std::exception & e )
    {
      itkWarningMacro( << e.what() );
      std::cout << "skipping series #" << i << " due to exception" << std::endl;
      reader->ResetPipeline();
    }
  }
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

      if ( image->GetPixelType().GetNumberOfComponents() == 1 )
      {
        SetDefaultImageProperties(node);
        // add level-window property
        //mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
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
  node->SetProperty( "volumerendering", mitk::BoolProperty::New( false ) );
  node->SetProperty( "use color", mitk::BoolProperty::New( true ) );
  node->SetProperty( "texture interpolation", mitk::BoolProperty::New( mitk::DataTreeNodeFactory::m_TextureInterpolationActive ) );	// set to user configurable default value (see global options)
  node->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->SetProperty( "layer", mitk::IntProperty::New(0));
  node->SetProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->SetOpacity(1.0f);
  node->SetColor(1.0,1.0,1.0);  
  node->SetVisibility(true);

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull())
  {
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( image );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

    // we adopt properties for the dataTreeNode if they are provided by our specific file i/o reader 
    // in form of image properties
    node->ConcatenatePropertyList(image->GetPropertyList());
  }

  // add a default rainbow lookup table for color mapping
  if(!node->GetProperty("LookupTable"))
  {
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
    vtkLut->SetHueRange(0.6667, 0.0);
    vtkLut->SetTableRange(0.0, 20.0);
    vtkLut->Build();
    mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
    mitkLutProp->SetLookupTable(mitkLut);
    node->SetProperty( "LookupTable", mitkLutProp );
  }
  if(!node->GetProperty("binary"))
    node->SetProperty( "binary", mitk::BoolProperty::New( false ) );
 
  // add a default transfer function
  mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
  if(image.IsNotNull() && !node->GetProperty("TransferFunction"))
  {

    /* //old Transfer Function Version
    float m_Min = image->GetScalarValueMin();
    float m_Max = image->GetScalarValueMax();

    tf->GetScalarOpacityFunction()->Initialize();
    tf->GetScalarOpacityFunction()->AddPoint ( m_Min, 0 );
    tf->GetScalarOpacityFunction()->AddPoint ( m_Max, 1 );
    tf->GetColorTransferFunction()->AddRGBPoint(m_Min,1,0,0);
    tf->GetColorTransferFunction()->AddRGBPoint(m_Max,1,1,0);
    tf->GetGradientOpacityFunction()->Initialize();
    tf->GetGradientOpacityFunction()->AddPoint(m_Min,1.0);
    tf->GetGradientOpacityFunction()->AddPoint(0.0,1.0);
    tf->GetGradientOpacityFunction()->AddPoint((m_Max*0.25),1.0);
    tf->GetGradientOpacityFunction()->AddPoint(m_Max,1.0);  
    */

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION >= 2)  )

    //CT_AAA for VTK Version >= 5.2
    tf->GetColorTransferFunction()->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314, 0.5, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196, 0.5, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 214.389, 1, 1, 1, 0.5, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 419.736, 1, 0.937033, 0.954531, 0.5, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 3071, 0.827451, 0.658824, 1, 0.5, 0 );

    tf->GetScalarOpacityFunction()->Initialize();
    tf->GetScalarOpacityFunction()->AddPoint( -3024, 0, 0.5, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 143.556, 0, 0.5, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 166.222, 0.686275, 0.5, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 214.389, 0.696078, 0.5, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 419.736, 0.833333, 0.5, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 3071, 0.803922, 0.5, 0 );

    tf->GetGradientOpacityFunction()->Initialize();
    tf->GetGradientOpacityFunction()->AddPoint( 0, 1, 0.5, 0 );
    tf->GetGradientOpacityFunction()->AddPoint( 255, 1, 0.5, 0);

#else

    //CT_AAA for VTK Version < 5.2
    tf->GetColorTransferFunction()->AddRGBPoint( -3024, 0, 0, 0 );
    tf->GetColorTransferFunction()->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314 );
    tf->GetColorTransferFunction()->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196 );
    tf->GetColorTransferFunction()->AddRGBPoint( 214.389, 1, 1, 1 );
    tf->GetColorTransferFunction()->AddRGBPoint( 419.736, 1, 0.937033, 0.954531 );
    tf->GetColorTransferFunction()->AddRGBPoint( 3071, 0.827451, 0.658824, 1 );

    tf->GetScalarOpacityFunction()->Initialize();
    tf->GetScalarOpacityFunction()->AddPoint( -3024, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 143.556, 0 );
    tf->GetScalarOpacityFunction()->AddPoint( 166.222, 0.686275 );
    tf->GetScalarOpacityFunction()->AddPoint( 214.389, 0.696078 );
    tf->GetScalarOpacityFunction()->AddPoint( 419.736, 0.833333 );
    tf->GetScalarOpacityFunction()->AddPoint( 3071, 0.803922 );

    tf->GetGradientOpacityFunction()->Initialize();
    tf->GetGradientOpacityFunction()->AddPoint( 0, 1 );
    tf->GetGradientOpacityFunction()->AddPoint( 255, 1 );

#endif

    node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );
  }
} 

void mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(mitk::DataTreeNode::Pointer &node)
{
  node->SetProperty( "line width", mitk::IntProperty::New(2) );
  node->SetProperty( "layer", mitk::IntProperty::New(1)); // Normally, surfaces are supposed to overlay images (on layer 0).
  node->SetProperty( "material", mitk::MaterialProperty::New( 1.0, 1.0, 1.0, 1.0, node.GetPointer() ) );
  node->SetProperty( "scalar visibility", mitk::BoolProperty::New(false) );
  node->SetProperty( "color mode", mitk::BoolProperty::New(false) );
  node->SetProperty( "representation", mitk::VtkRepresentationProperty::New() );
  node->SetProperty( "interpolation", mitk::VtkInterpolationProperty::New() );
  node->SetProperty( "scalar mode", mitk::VtkScalarModeProperty::New() );
  node->SetVisibility(true);
  mitk::Surface::Pointer surface = dynamic_cast<Surface*>(node->GetData());
  if(surface.IsNotNull())
  {
    if((surface->GetVtkPolyData() != 0) && (surface->GetVtkPolyData()->GetPointData() != 0) && (surface->GetVtkPolyData()->GetPointData()->GetScalars() != 0))
    {
      node->SetProperty( "scalar visibility", mitk::BoolProperty::New(true) );
      node->SetProperty( "color mode", mitk::BoolProperty::New(true) );
    }
  }
}

void mitk::DataTreeNodeFactory::SetDefaultSegmentationProperties(DataTreeNode::Pointer &node)
{
  node->SetProperty( "volumerendering", mitk::BoolProperty::New( false ) );
  node->SetProperty( "use color", mitk::BoolProperty::New( true ) );
  node->SetProperty( "texture interpolation", mitk::BoolProperty::New( false ) );
  node->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->SetProperty( "layer", mitk::IntProperty::New(1));
  node->SetProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->SetProperty( "binary", mitk::BoolProperty::New(true) );
  node->SetProperty( "segmentation", mitk::BoolProperty::New(true) );
  node->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  node->SetProperty( "color" , mitk::ColorProperty::New( 1.0f, 0.0f, 0.0f ));
  node->SetOpacity(0.3f);
  node->SetVisibility(true);
}

void mitk::DataTreeNodeFactory::SetDefaultUnstructuredGridProperties(DataTreeNode::Pointer &node)
{
  node->SetProperty( "line width", mitk::IntProperty::New(1));
  node->SetProperty( "layer", mitk::IntProperty::New(0));
  node->SetProperty( "material", mitk::MaterialProperty::New( 1.0, 1.0, 1.0, 1.0, node.GetPointer()));
  node->SetProperty( "grid representation", mitk::GridRepresentationProperty::New());
  node->SetProperty( "grid volume mapper", mitk::GridVolumeMapperProperty::New());
  node->SetProperty( "interpolation", mitk::VtkInterpolationProperty::New() );
  node->SetProperty( "scalar visibility", mitk::BoolProperty::New(true) );
  node->SetProperty( "scalar mode", mitk::VtkScalarModeProperty::New() );
  node->SetVisibility(true);
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

void mitk::DataTreeNodeFactory::SetDefaultPointSetProperties(mitk::DataTreeNode::Pointer &node)
{
  node->SetProperty( "line width", mitk::IntProperty::New(2) );
  node->SetProperty( "layer", mitk::IntProperty::New(1));
  node->SetProperty( "pointsize", mitk::FloatProperty::New(1.0));
  node->SetProperty( "unselectedcolor", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f));//yellow
  node->SetProperty( "selectedcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));//red
  node->SetProperty( "show contour", mitk::BoolProperty::New(false) );
  node->SetProperty( "contourcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));
  node->SetProperty( "close contour", mitk::BoolProperty::New(false) );

  //node->SetProperty( "material", mitk::MaterialProperty::New( 1.0, 1.0, 1.0, 1.0, node.GetPointer() ) );
  node->SetVisibility(true);
}


void mitk::DataTreeNodeFactory::SetDefaultCommonProperties(mitk::DataTreeNode::Pointer &node)
{
  // path
  mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenamePath( m_FileName ) );
  node->SetProperty( StringProperty::PATH, pathProp );
  
  // name
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
  if(nameProp.IsNull() || (strcmp(nameProp->GetValue(),"No Name!")==0))
  {
    if (FileNameEndsWith( ".gz" ))
      m_FileName = m_FileName.substr( 0, m_FileName.length()-3 );
    
    nameProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenameWithoutLastExtension( m_FileName ) );

    node->SetProperty( "name", nameProp );
  }
  
  // visibility
  if(!node->GetProperty("visible"))
    node->SetVisibility(true);
}


