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


#include "mitkDataTreeNodeFactory.h"

// C-Standard library includes
#include <stdlib.h>

// STL-related includes
#include <vector>
#include <map>

// VTK-related includes
#include <vtkSTLReader.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>

// ITK-related includes
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkDICOMImageIO2.h>
#include <itkDICOMSeriesFileNames.h>
#include <itkImageFileReader.h>
#include <itkImageIOFactory.h>
#include <itkImageIORegion.h>
#include <itkImageSeriesReader.h>
#include <itkDICOMImageIO2.h>
#include <itkDICOMSeriesFileNames.h>
#include <itkNumericSeriesFileNames.h>

// MITK-related includes
#include "mitkSurface.h"
#include "mitkPicFileReader.h"
#include "mitkPicVolumeTimeSeriesReader.h"
#include "mitkStringProperty.h"
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"
#include "mitkParRecFileReader.h"
#include "mitkImage.h"
#include "mitkUSLookupTableSource.h"
#include "mitkLookupTableProperty.h"
#include "mitkImageChannelSelector.h"
#include "mitkImageSliceSelector.h"
#ifdef MBI_INTERNAL
 #include "mitkVesselGraphFileReader.h"
 #include "mitkVesselTreeFileReader.h"
 #include "mitkDICOMFileReader.h"
 #include "mitkCylindricToCartesianFilter.h"
 #include "mitkDSRFileReader.h"
#endif





mitk::DataTreeNodeFactory::DataTreeNodeFactory()
{
    this->Modified();
}



mitk::DataTreeNodeFactory::~DataTreeNodeFactory()
{}




void mitk::DataTreeNodeFactory::GenerateData()
{
    if ( m_FileName != "" )
    {
        if ( this->FileNameEndsWith( ".stl" ) )
        {
            this->ReadFileTypeSTL();
        }
        else if ( this->FileNameEndsWith( ".vtk" ) )
        {
            this->ReadFileTypeVTK();
        }
        else if ( this->FileNameEndsWith( ".pic" ) || this->FileNameEndsWith( ".pic.gz" ) || this->FileNameEndsWith( ".seq" ) )
        {
            this->ReadFileTypePIC();
        }
        else if ( this->FileNameEndsWith( ".par" ) )
        {
            this->ReadFileTypePAR();
        }
        else if ( this->FileNameEndsWith( ".pvtk" ) )
        {
            this->ReadFileTypePVTK();
        }
#ifdef MBI_INTERNAL
        else if ( this->FileNameEndsWith( ".DCM" ) || this->FileNameEndsWith( ".dcm" ) )
        {
            this->ReadFileTypeDCM();
        }
        else if ( this->FileNameEndsWith( ".ves" ) )
        {
            this->ReadFileTypeVES();
        }
        else if ( this->FileNameEndsWith( ".uvg" ) )
        {
            this->ReadFileTypeUVG();
        }
        else if ( this->FileNameEndsWith( ".dvg" ) )
        {
            this->ReadFileTypeDVG();
        }
        else if ( this->FileNameEndsWith( "HPSONOS.DB" ) || this->FileNameEndsWith( "hpsonos.db" ) )
        {
            this->ReadFileTypeHPSONOS();
        }
#endif
        //else if (this->FileNameEndsWith("."))
        //{
        //    //put your new filetype in here!
        //}
        else
        {
            this->ReadFileTypeITKImageIOFactory();
        }
    }
    else if ( m_FilePattern != "" && m_FilePrefix != "" )
    {
        if ( this->FilePatternEndsWith( ".pic" ) || this->FilePatternEndsWith( ".pic.gz" ) )
        {
            this->ReadFileSeriesTypePIC();
        }
        else if ( this->FilePatternEndsWith( ".dcm" ) || this->FilePatternEndsWith( ".DCM" ) )
        {
            this->ReadFileSeriesTypeDCM();
        }
        else if ( this->FilePatternEndsWith( ".png" ) || this->FilePatternEndsWith( ".PNG" ) )
        {
            this->ReadFileSeriesTypeITKImageSeriesReader();
        }
        else if ( this->FilePatternEndsWith( ".stl" ) || this->FilePatternEndsWith( ".STL" ) )
        {
            this->ReadFileSeriesTypeSTL();
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




void mitk::DataTreeNodeFactory::ReadFileTypeSTL()
{
    std::cout << "Loading " << m_FileName << " as stl..." << std::endl;

    vtkSTLReader* stlReader = vtkSTLReader::New();
    stlReader->SetFileName( m_FileName.c_str() );
    stlReader->Update();

    if ( stlReader->GetOutput() != NULL )
    {
        mitk::Surface::Pointer surface = mitk::Surface::New();
        surface->SetVtkPolyData( stlReader->GetOutput() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( surface );

        // set filename without path as string property
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
        node->SetProperty( "lineWidth", new mitk::IntProperty(2) );
        node->SetProperty( "name", nameProp );
        node->SetProperty( "wireframe", new mitk::BoolProperty(false));
    }

    stlReader->Delete();

    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypeVTK()
{
    std::cout << "Loading " << m_FileName << " as vtk..." << std::endl;

    vtkPolyDataReader *reader = vtkPolyDataReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();

    if ( reader->GetOutput() != NULL )
    {
        mitk::Surface::Pointer surface = mitk::Surface::New();
        surface->SetVtkPolyData( reader->GetOutput() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( surface );

        // set filename without path as string property
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
        node->SetProperty( "name", nameProp );
        node->SetProperty( "wireframe", new mitk::BoolProperty(false));
    }

    reader->Delete();

    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypePIC()
{
    std::cout << "Loading " << m_FileName << " as pic... " << std::endl;
    ipPicDescriptor * header = ipPicGetHeader( const_cast<char *>( m_FileName.c_str() ), NULL );
    if ( header != NULL )
    {
        mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();

        reader->SetFileName( m_FileName.c_str() );
        reader->UpdateLargestPossibleRegion();
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( reader->GetOutput() );

        // set filename without path as string property
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
        node->SetProperty( "name", nameProp );


        SetDefaultImageProperties(node);

        // add level-window property
        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;

        levelwindow.SetAuto( reader->GetOutput()->GetPic() );
        levWinProp->SetLevelWindow( levelwindow );

        node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
        
    }
    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypePAR()
{
    std::cout << "loading " << m_FileName << " as par/rec ... " << std::endl;

    mitk::ParRecFileReader::Pointer reader = mitk::ParRecFileReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );


    SetDefaultImageProperties(node);

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );

    // add level-window property
    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput()->GetPic() );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
    
    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypePVTK()
{
    vtkStructuredPointsReader * vtkreader = vtkStructuredPointsReader::New();
    vtkreader->SetFileName( m_FileName.c_str() );
    vtkreader->Update();

    if ( vtkreader->GetOutput() != NULL )
    {
        mitk::Image::Pointer image = mitk::Image::New();
        image->Initialize( vtkreader->GetOutput() );
        image->SetVolume( vtkreader->GetOutput()->GetScalarPointer() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( image );


        SetDefaultImageProperties(node);

        // set filename without path as string property
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
        node->SetProperty( "name", nameProp );

        // add level-window property
        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;
        levelwindow.SetAuto( image->GetPic() );
        levWinProp->SetLevelWindow( levelwindow );
        node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
        

    }

    vtkreader->Delete();

}

#ifdef MBI_INTERNAL



void mitk::DataTreeNodeFactory::ReadFileTypeVES()
{
    std::cout << "Loading " << m_FileName << " as ves... " << std::endl;

    mitk::VesselTreeFileReader::Pointer reader = mitk::VesselTreeFileReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );

    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypeUVG()
{
    std::cout << "Loading " << m_FileName << " as uvg... " << std::endl;

    mitk::VesselGraphFileReader<Undirected>::Pointer reader = mitk::VesselGraphFileReader<Undirected>::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );

    std::cout << "...finished!" << std::endl;
}

void mitk::DataTreeNodeFactory::ReadFileTypeDVG()
{
    std::cout << "Loading " << m_FileName << " as dvg... " << std::endl;

    mitk::VesselGraphFileReader<Directed>::Pointer reader = mitk::VesselGraphFileReader<Directed>::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );

    std::cout << "...finished!" << std::endl;
}


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
    node->SetProperty( "layer", new mitk::IntProperty( -11 ) );
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( sliceSelector->GetOutput()->GetPic() );
    node->SetLevelWindow( levelwindow, NULL );
    node->SetVisibility( false, NULL );
    node->SetColor( 1.0, 1.0, 1.0, NULL );
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
    node->SetProperty( "layer", new mitk::IntProperty( -10 ) );

    node->SetLevelWindow( levelwindow, NULL );
    node->SetColor( 1.0, 1.0, 1.0, NULL );
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
        mitk::LookupTableProperty::Pointer LookupTableProp = new mitk::LookupTableProperty( *LookupTable );

				int start = 128 - 1;
				int end = 128 + 1;
				for (int i=start; i<=end;i++)
					LookupTableProp->GetLookupTable().ChangeOpacity(i, 0);

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
        node->SetProperty( "layer", new mitk::IntProperty( -6 ) );

        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;
        levelwindow.SetLevelWindow( 128, 255 );
        levWinProp->SetLevelWindow( levelwindow );
        // set the overwrite LevelWindow
        // if "levelwindow" is used if "levelwindow" is not available
        // else "levelwindow" is used
        // "levelwindow" is not affected by the slider
        node->GetPropertyList()->SetProperty( "levelWindow", levWinProp );

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
        node->SetProperty( "layer", new mitk::IntProperty( -5 ) );

        // set the overwrite LevelWindow
        // if "levelwindow" is used if "levelwindow" is not available
        // else "levelwindow" is used
        // "levelwindow" is not affected by the slider
        node->GetPropertyList()->SetProperty( "levelWindow", levWinProp );

        node->SetProperty( "LookupTable", LookupTableProp );
        node->Update();


        SetDefaultImageProperties(node);
    }
    std::cout << "...finished!" << std::endl;
}


void mitk::DataTreeNodeFactory::ReadFileTypeDCM()
{
    std::cout << "loading " << m_FileName << " as DICOM... " << std::endl;

    mitk::DICOMFileReader::Pointer reader = mitk::DICOMFileReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );


    SetDefaultImageProperties(node);


    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );

    // add Level-Window property
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput()->GetPic() );
    node->SetLevelWindow( levelwindow, NULL );

    
    std::cout << "...finished!" << std::endl;
}


#endif



void mitk::DataTreeNodeFactory::ReadFileTypeITKImageIOFactory()
{
    const unsigned int MINDIM = 2;
    const unsigned int MAXDIM = 4;

    std::cout << "loading " << m_FileName << " via itk::ImageIOFactory... " << std::endl;

    // Check to see if we can read the file given the name or prefix
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "File Type not supported!" );
        return ;
    }

    itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO( m_FileName.c_str(), itk::ImageIOFactory::ReadMode );
    if ( imageIO.IsNull() )
    {
        itkWarningMacro( << "File Type not supported!" );
        return ;
    }

    // Got to allocate space for the image. Determine the characteristics of
    // the image.
    imageIO->SetFileName( m_FileName.c_str() );
    imageIO->ReadImageInformation();

    unsigned int ndim = imageIO->GetNumberOfDimensions();
    if ( ndim < MINDIM || ndim > MAXDIM )
    {
        itkWarningMacro( << "Sorry, only dimensions 2, 3 and 4 are supported. The given file has " << ndim << " dimensions!" );
        return ;
    }

    itk::ImageIORegion ioRegion( ndim );
    itk::ImageIORegion::SizeType ioSize = ioRegion.GetSize();
    itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();

    unsigned int dimensions[ MAXDIM ];
    dimensions[ 0 ] = 0;
    dimensions[ 1 ] = 0;
    dimensions[ 2 ] = 0;
    dimensions[ 3 ] = 0;

    float spacing[ MAXDIM ];
    spacing[ 0 ] = 1.0f;
    spacing[ 1 ] = 1.0f;
    spacing[ 2 ] = 1.0f;
    spacing[ 3 ] = 1.0f;

    for ( unsigned int i = 0; i < ndim ; ++i )
    {
        ioStart[ i ] = 0;
        ioSize[ i ] = imageIO->GetDimensions( i );
        dimensions[ i ] = imageIO->GetDimensions( i );
        spacing[ i ] = imageIO->GetSpacing( i );
    }

    ioRegion.SetSize( ioSize );
    ioRegion.SetIndex( ioStart );

    std::cout << "ioRegion: " << ioRegion << std::endl;
    imageIO->SetIORegion( ioRegion );


    void* buffer = malloc( imageIO->GetImageSizeInBytes() );
    imageIO->Read( buffer );
    mitk::Image::Pointer image = mitk::Image::New();
#if ITK_VERSION_MAJOR == 2 || ( ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR > 6 )
    mitk::PixelType pixelType( imageIO->GetComponentTypeInfo(), imageIO->GetNumberOfComponents() );
    image->Initialize( pixelType, ndim, dimensions );
#else
    if ( imageIO->GetNumberOfComponents() > 1)
        itkWarningMacro(<<"For older itk versions, only scalar images are supported!");
    mitk::PixelType pixelType( imageIO->GetPixelType() );
    image->Initialize( pixelType, ndim, dimensions );
#endif 
    image->SetVolume( buffer );
    image->GetSlicedGeometry()->SetSpacing( spacing );
    image->GetTimeSlicedGeometry()->InitializeEvenlyTimed(image->GetSlicedGeometry(), image->GetDimension(3));
    free( buffer );
    buffer = NULL;
    std::cout << "number of image components: "<< image->GetPixelType().GetNumberOfComponents() << std::endl;
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( image );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "name", nameProp );
    std::cout << "...finished!" << std::endl;
}

void mitk::DataTreeNodeFactory::ReadFileSeriesTypePIC()
{
    std::cout << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as pic..." << std::endl;

    mitk::PicVolumeTimeSeriesReader::Pointer reader;
    reader = mitk::PicVolumeTimeSeriesReader::New();
    reader->SetFilePrefix( m_FilePrefix.c_str() );
    reader->SetFilePattern( m_FilePattern.c_str() );
    reader->UpdateLargestPossibleRegion();

    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    std::string filename = this->GetBaseFilePrefix() + "pic";
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( filename );
    node->SetProperty( "name", nameProp );


    SetDefaultImageProperties(node);

    // add level-window property
    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;

    levelwindow.SetAuto( reader->GetOutput()->GetPic() );
    levWinProp->SetLevelWindow( levelwindow );

    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
    
    std::cout << "...finished!" << std::endl;
}

void mitk::DataTreeNodeFactory::ReadFileSeriesTypeDCM()
{
  std::cout << "loading image series with prefix " << m_FilePrefix << " and pattern " << m_FilePattern << " as DICOM..." << std::endl;

  typedef itk::Image<int, 3> ImageType;
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

      // add level-window property
      mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
      mitk::LevelWindow levelwindow;
      levelwindow.SetAuto( image->GetPic() );
      levWinProp->SetLevelWindow( levelwindow );
      node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

      

      // set filename without path as string property
      std::string filename = std::string( this->GetBaseFilePrefix() );
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( seriesUID[ i ] );
      node->SetProperty( "name", nameProp );

    }
    catch ( const std::exception & e )
    {
      itkWarningMacro( << e.what() );
      return ;
    }
  }
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
        itkWarningMacro( << "Sorry, none of the files matched the prefix!" )
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
        for ( unsigned int i = baseFilename.length() - 1; i >= 0; --i )
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
        itkWarningMacro( << "Sorry, no numbered files found, I can't load anything..." )
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


            SetDefaultImageProperties(node);

            // add level-window property
            mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
            mitk::LevelWindow levelwindow;

            levelwindow.SetAuto( image->GetPic() );
            levWinProp->SetLevelWindow( levelwindow );

            node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
    
        }
    }
    catch ( const std::exception & e )
    {
        itkWarningMacro( << e.what() );
        return ;
    }
}


void mitk::DataTreeNodeFactory::ReadFileSeriesTypeSTL()
{
    if ( !this->GenerateFileList() )
    {
        itkWarningMacro( << "Sorry, file list could not be determined..." );
        return ;
    }

    mitk::Surface::Pointer surface = mitk::Surface::New();
    std::cout << "prefix: "<< m_FilePrefix << ", pattern: " <<m_FilePattern << std::endl;
    surface->Initialize(m_MatchedFileNames.size());
    //surface->Resize( m_MatchedFileNames.size() );
    for ( unsigned int i = 0 ; i < m_MatchedFileNames.size(); ++i )
    {
        std::string fileName = m_MatchedFileNames[i];
        std::cout << "Loading " << fileName << " as stl..." << std::endl;

        vtkSTLReader* stlReader = vtkSTLReader::New();
        stlReader->SetFileName( fileName.c_str() );
        stlReader->Update();

        if ( stlReader->GetOutput() != NULL )
        {
            surface->SetVtkPolyData( stlReader->GetOutput(), i );
        }
        else
        {
            itkWarningMacro(<< "stlReader returned NULL while reading " << fileName << ". Trying to continue with empty vtkPolyData...");
            surface->SetVtkPolyData( vtkPolyData::New(), i ); 
        }
        stlReader->Delete();
    }
    
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( surface );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFilePrefix() + ".stl" );
    node->SetProperty( "name", nameProp );
    std::cout << "...finished!" << std::endl;
}

void mitk::DataTreeNodeFactory::SetDefaultImageProperties(mitk::DataTreeNode::Pointer &node) {
        node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
        node->SetProperty( "iilInterpolation", new mitk::BoolProperty( false ) );
        node->SetProperty( "vtkInterpolation", new mitk::BoolProperty( false ) );
}; 
