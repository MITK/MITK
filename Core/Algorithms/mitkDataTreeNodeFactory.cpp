#include "mitkDataTreeNodeFactory.h"

// STL-related includes
#include <vector>

// VTK-related includes
#include <vtkSTLReader.h>
#include <vtkPolyDataReader.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>

// ITK-related includes
#include <itksys/SystemTools.hxx>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkDICOMImageIO2.h>
#include <itkDICOMSeriesFileNames.h>

// MITK-related includes
#include "mitkSurfaceData.h"
#include "mitkPicFileReader.h"
#include "mitkStringProperty.h"
#include "mitkProperties.h"
#include "mitkBoolProperty.h"
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
    if ( this->FileNameEndsWith( ".stl" ) )
    {
        this->ReadFileTypeSTL();
    }
    else if ( this->FileNameEndsWith( ".vtk" ) )
    {
        this->ReadFileTypeVTK();
    }
    else if ( this->FileNameEndsWith( ".pic" ) || this->FileNameEndsWith( ".seq" ) )
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
    else if ( this->FileNameEndsWith( ".DCM" ) || this->FileNameEndsWith( ".dcm" ) )
    {
        this->ReadFileTypeDCM();
    }
#ifdef MBI_INTERNAL
    else if ( this->FileNameEndsWith( ".ves" ) )
    {
        this->ReadFileTypeVES();
    }
    else if ( this->FileNameEndsWith( ".uvg" ) )
    {
        this->ReadFileTypeUVG();
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
}




bool mitk::DataTreeNodeFactory::FileNameEndsWith( const std::string& name )
{
    return m_FileName.find( name ) != std::string::npos;
}



std::string mitk::DataTreeNodeFactory::GetBaseFileName()
{
    return itksys::SystemTools::GetFilenameName( m_FileName );
}



std::string mitk::DataTreeNodeFactory::GetDirectory()
{
    return itksys::SystemTools::GetFilenamePath( m_FileName );
}




void mitk::DataTreeNodeFactory::ReadFileTypeSTL()
{
    std::cout << "Loading " << m_FileName << " as stl..." << std::endl;

    vtkSTLReader* stlReader = vtkSTLReader::New();
    stlReader->SetFileName( m_FileName.c_str() );
    stlReader->Update();

    if ( stlReader->GetOutput() != NULL )
    {
        mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
        surface->SetVtkPolyData( stlReader->GetOutput() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( surface );
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
        mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
        surface->SetVtkPolyData( reader->GetOutput() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( surface );
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
        node->SetProperty( "fileName", nameProp );

        // disable volume rendering by default
        node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

        // add level-window property
        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;

        levelwindow.SetAuto( reader->GetOutput() ->GetPic() );
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

    // disable volume rendering by default
    node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "fileName", nameProp );

    // add level-window property
    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput() ->GetPic() );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList() ->SetProperty( "levelwindow", levWinProp );

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
        image->SetVolume( vtkreader->GetOutput() ->GetScalarPointer() );
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( image );

        // disable volume rendering by default
        node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

        // set filename without path as string property
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
        node->SetProperty( "fileName", nameProp );

        // add level-window property
        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;
        levelwindow.SetAuto( image->GetPic() );
        levWinProp->SetLevelWindow( levelwindow );
        node->GetPropertyList() ->SetProperty( "levelwindow", levWinProp );
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
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "fileName", nameProp );

    std::cout << "...finished!" << std::endl;
}




void mitk::DataTreeNodeFactory::ReadFileTypeUVG()
{
    std::cout << "Loading " << m_FileName << " as uvg... " << std::endl;

    mitk::VesselGraphFileReader::Pointer reader = mitk::VesselGraphFileReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
        mitk::DataTreeNode::Pointer node = this->GetOutput();
        node->SetData( reader->GetOutput() );
    node->SetData( reader->GetOutput() );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "fileName", nameProp );

    std::cout << "...finished!" << std::endl;
}

void mitk::DataTreeNodeFactory::ReadFileTypeHPSONOS()
{
    std::cout << "loading " << m_FileName << " as DSR ... " << std::endl;

    this->SetNumberOfOutputs( 2 );
    this->SetNthOutput(0, this->MakeOutput(0).GetPointer());
    this->SetNthOutput(1, this->MakeOutput(1).GetPointer());
    

    mitk::DSRFileReader::Pointer reader;

    reader = mitk::DSRFileReader::New();

    reader->SetFileName( m_FileName.c_str() );

    mitk::ImageChannelSelector::Pointer channelSelector = mitk::ImageChannelSelector::New();
    mitk::ImageChannelSelector::Pointer DopplerChannelSelector = mitk::ImageChannelSelector::New();

    channelSelector->SetInput( reader->GetOutput() );
    DopplerChannelSelector->SetInput( reader->GetOutput() );

    reader->UpdateOutputInformation();

    bool haveDoppler = false;
    if ( reader->GetOutput() ->IsValidChannel( 0 ) )
    {
        std::cout << "    have channel data 0 (backscatter) ... " << std::endl;
    }

    if ( reader->GetOutput() ->IsValidChannel( 1 ) )
    {
        std::cout << "    have channel data 1 (doppler) ... " << std::endl;
        haveDoppler = true;
    }

    mitk::CylindricToCartesianFilter::Pointer cyl2cart = mitk::CylindricToCartesianFilter::New();
    mitk::CylindricToCartesianFilter::Pointer cyl2cartDoppler = mitk::CylindricToCartesianFilter::New();

    cyl2cart->SetTargetXSize( 128 );
    cyl2cartDoppler->SetTargetXSize( 128 );

    //
    // switch to Backscatter information
    //
    channelSelector->SetChannelNr( 0 );

    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
    //
    // insert original (in cylinric coordinates) Backscatter information
    //
    mitk::DataTreeNode::Pointer node = this->GetOutput(0);
    node->SetData( channelSelector->GetOutput() );
    node->SetData( channelSelector->GetOutput() );
    sliceSelector->SetInput( channelSelector->GetInput() );
    mitk::StringProperty::Pointer ultrasoundProp = new mitk::StringProperty( "OriginalBackscatter" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( "OriginalBackscatter" );
    node->SetProperty( "fileName", nameProp );
    node->SetProperty( "layer", new mitk::IntProperty( -11 ) );
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( sliceSelector->GetOutput() ->GetPic() );
    node->SetLevelWindow( levelwindow, NULL );
    node->SetVisibility( false, NULL );
    node->Update();
    // disable volume rendering by default
    node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

    //
    // insert transformed (in cartesian coordinates) Backscatter information
    //
    cyl2cart->SetInput( reader->GetOutput() );
    node = this->GetOutput(1);
    node->SetData(reader->GetOutput());
    node->SetData( cyl2cart->GetOutput() );
    ultrasoundProp = new mitk::StringProperty( "TransformedBackscatter" );
    node->SetProperty( "ultrasound", ultrasoundProp );
    nameProp = new mitk::StringProperty( "TransformedBackscatter" );
    node->SetProperty( "fileName", nameProp );
    node->SetProperty( "layer", new mitk::IntProperty( -10 ) );
    //    mitk::LevelWindow levelwindow;

    sliceSelector->SetInput( cyl2cart->GetOutput() );
    sliceSelector->Update();
    levelwindow.SetAuto( sliceSelector->GetOutput() ->GetPic() );
    node->SetLevelWindow( levelwindow, NULL );
    node->Update();

    // disable volume rendering by default
    node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

    if ( haveDoppler )
    {
        this->SetNumberOfOutputs( 4 );
        this->SetNthOutput(2, this->MakeOutput(2).GetPointer());
        this->SetNthOutput(3, this->MakeOutput(3).GetPointer());

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


        //
        // insert original (in cylindric coordinates) Doppler information
        //
        node = this->GetOutput(2);
        node->SetData(DopplerChannelSelector->GetOutput());
        node->SetData( DopplerChannelSelector->GetOutput() );
        ultrasoundProp = new mitk::StringProperty( "OriginalDoppler" );
        node->SetProperty( "ultrasound", ultrasoundProp );
        nameProp = new mitk::StringProperty( "OriginalDoppler" );
        node->SetProperty( "fileName", nameProp );
        node->SetProperty( "layer", new mitk::IntProperty( -6 ) );


        mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
        mitk::LevelWindow levelwindow;
        levelwindow.SetLevelWindow( 128, 255 );
        levWinProp->SetLevelWindow( levelwindow );
        // set the overwrite LevelWindow
        // if "levelwindow" is used if "levelwindow" is not available
        // else "levelwindow" is used
        // "levelwindow" is not affected by the slider
        node->GetPropertyList() ->SetProperty( "levelWindow", levWinProp );

        node->SetProperty( "LookupTable", LookupTableProp );
        node->SetVisibility( false, NULL );
        node->Update();

        // disable volume rendering by default
        node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

        //
        // insert transformed (in cartesian coordinates) Doppler information
        //
        cyl2cartDoppler->SetInput( DopplerChannelSelector->GetOutput() );

        node = this->GetOutput(3);
        node->SetData( cyl2cartDoppler->GetOutput() );
        node->SetData( cyl2cartDoppler->GetOutput() );
        ultrasoundProp = new mitk::StringProperty( "TransformedDoppler" );
        node->SetProperty( "ultrasound", ultrasoundProp );
        nameProp = new mitk::StringProperty( "TransformedDoppler" );
        node->SetProperty( "fileName", nameProp );
        node->SetProperty( "layer", new mitk::IntProperty( -5 ) );
        cyl2cartDoppler->Update();

        // set the overwrite LevelWindow
        // if "levelwindow" is used if "levelwindow" is not available
        // else "levelwindow" is used
        // "levelwindow" is not affected by the slider
        node->GetPropertyList() ->SetProperty( "levelWindow", levWinProp );

        node->SetProperty( "LookupTable", LookupTableProp );
        node->Update();

        // disable volume rendering by default
        node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
    }
    std::cout << "...finished!" << std::endl;
}



#endif



void mitk::DataTreeNodeFactory::ReadFileTypeDCM()
{
    std::cout << "loading " << m_FileName << " as DICOM... " << std::endl;

#ifdef MBI_INTERNAL

    mitk::DICOMFileReader::Pointer reader = mitk::DICOMFileReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();
    mitk::DataTreeNode::Pointer node = this->GetOutput();
    node->SetData( reader->GetOutput() );

    // disable volume rendering by default
    node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );

    // set filename without path as string property
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty( this->GetBaseFileName() );
    node->SetProperty( "fileName", nameProp );

    // add Level-Window property
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput() ->GetPic() );
    node->SetLevelWindow( levelwindow, NULL );

#else
    /*
        std::string dir = this->GetDirectory();
     
        typedef itk::Image<unsigned short, 3> ImageType;
        typedef itk::ImageSeriesReader< ImageType > ReaderType;
     
        itk::DICOMImageIO2::Pointer dicomIO = itk::DICOMImageIO2::New();
     
        // Get the DICOM filenames from the directory
        itk::DICOMSeriesFileNames::Pointer nameGenerator = itk::DICOMSeriesFileNames::New();
        nameGenerator->SetDirectory( dir.c_str() );
     
        typedef std::vector<std::string> seriesIdContainer;
        const seriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
     
        seriesIdContainer::const_iterator seriesItr = seriesUID.begin();
        seriesIdContainer::const_iterator seriesEnd = seriesUID.end();
     
        std::cout << "The directory: " << dir << " contains the following DICOM Series: " << std::endl;
        while ( seriesItr != seriesEnd )
        {
            std::cout << seriesItr->c_str() << std::endl;
            seriesItr++;
        }
     
        std::cout << std::endl << "Now reading series: " << std::endl << std::endl;
     
        typedef std::vector<std::string> fileNamesContainer;
        fileNamesContainer fileNames;
     
        std::cout << seriesUID.begin() ->c_str() << std::endl;
        fileNames = nameGenerator->GetFileNames();
     
        ReaderType::Pointer reader = ReaderType::New();
        reader->SetFileNames( fileNames );
        reader->SetImageIO( dicomIO );
     
        try
        {
            reader->Update();
        }
        catch ( itk::ExceptionObject & ex )
        {
            std::cout << ex.what() << std::endl;
            //return ;
        }
        it = tree->inorderIterator();
     
        CommonFunctionality::AddItkImageToDataTree<ImageType>( reader->GetOutput(), it, fileName );
     
     
        // disable volume rendering by default
        //    node->SetProperty("volumerendering",new mitk::BoolProperty(false));
     
        //mitk::LevelWindow levelwindow;
        //reader->Update();
        //node->SetLevelWindow(levelwindow, NULL);
        //mitkMultiWidget->levelwindowWidget->setLevelWindow(levelwindow);
     
    */
#endif

    std::cout << "...finished!" << std::endl;
}





