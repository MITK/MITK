#include "mitkPicVolumeTimeSeriesReader.h"
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <map>
#include <string>

bool mitk::PicVolumeTimeSeriesReader::GenerateFileList()
{
    typedef std::vector<std::string> StringContainer;
    typedef std::map<unsigned int, std::string> SortedStringContainer;
    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FilePrefix == "" || m_FilePattern == "" )
    {
        throw itk::ImageFileReaderException( __FILE__, __LINE__, "Both FilePattern and FilePrefix must be non-empty" );
    }

    //
    // Load Directory
    //
    std::string directory = itksys::SystemTools::GetFilenamePath( m_FilePrefix );
    itksys::Directory itkDir;
    if ( !itkDir.Load ( directory.c_str() ) )
    {
        itkWarningMacro ( << "Directory " << directory << " cannot be read!" );
        return false;
    }

    //
    // Get a list of all files in the directory
    //
    StringContainer unmatchedFiles;
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
    StringContainer matchedFiles;
    std::string prefix = itksys::SystemTools::GetFilenameName( m_FilePrefix );
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
        return false;
    }


    //
    // parse the file names from back to front for digits
    // and convert them to a number. Store the filename and number
    // in a SortedStringContainer
    //
    SortedStringContainer sortedFiles;
    for ( StringContainer::iterator it = matchedFiles.begin() ; it != matchedFiles.end() ; ++it )
    {
        //remove the last extension, until we have a digit at the end, or no extension is left!
        std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( *it );
        while ( ( baseFilename[baseFilename.length()-1] < '0' || baseFilename[baseFilename.length()-1] > '9') && itksys::SystemTools::GetFilenameLastExtension(baseFilename) != "") 
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
        return false;
    }

    //
    // Convert the sorted string container in a plain sorted vector of strings;
    //
    m_MatchedFiles.clear();
    m_MatchedFiles.resize( sortedFiles.size() );
    unsigned int i = 0;
    for ( SortedStringContainer::iterator it = sortedFiles.begin() ; it != sortedFiles.end() ; ++it, ++i )
    {
        m_MatchedFiles[ i ] = it->second ;
        itkDebugMacro( << "Added " << it->second << " to the set of matched files!" );
    }
    return true;
}




void mitk::PicVolumeTimeSeriesReader::GenerateOutputInformation()
{
    mitk::Image::Pointer output = this->GetOutput();

    if ( ( output->IsInitialized() ) && ( this->GetMTime() <= m_ReadHeaderTime.GetMTime() ) )
        return ;

    itkDebugMacro( << "Reading file for GenerateOutputInformation()" << m_FileName );

    if ( ! this->GenerateFileList() )
    {
        itkWarningMacro( "Sorry, file list could not be generated!" );
        return ;
    }

    //
    // Read the first file of the image sequence. Assume equal size and spacings for all
    // other volumes. @TODO Integrate support for different sizes and spacings
    //
    char* filename = const_cast<char *> ( m_MatchedFiles[ 0 ].c_str() );
    ipPicDescriptor * header = ipPicGetHeader( filename, NULL );
    header = ipPicGetTags( filename, header );

    if ( header == NULL )
    {
        itk::ImageFileReaderException e( __FILE__, __LINE__ );
        itk::OStringStream msg;
        msg << " Could not read file " << m_FileName.c_str();
        e.SetDescription( msg.str().c_str() );
        throw e;
        return ;
    }
    if ( header->dim != 3 )
    {
        itk::ImageFileReaderException e( __FILE__, __LINE__ , "Only 3D-pic volumes are supported! " );
        throw e;
        return ;
    }

    //
    // modify the header to match the real temporal extent
    //
    header->dim = 4;
    header->n[ 3 ] = m_MatchedFiles.size();

    output->Initialize( header );

    ipPicFree( header );
    
    //
    // Initialize the timebounds of the FIRST geometry/volume. 
    // Since EvenlyTimed is activated, the timings are propagated 
    // to the other volumes!
    //
    mitk::ScalarType timearray[ 2 ];
    timearray[ 0 ] = 0;
    timearray[ 1 ] = 1; 
    TimeBounds timebounds( timearray );
    output->GetSlicedGeometry()->SetTimeBoundsInMS( timebounds );

    m_ReadHeaderTime.Modified();
}


void mitk::PicVolumeTimeSeriesReader::GenerateData()
{
    mitk::Image::Pointer output = this->GetOutput();

    //
    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FilePrefix == "" || m_FilePattern == "" )
    {
        throw itk::ImageFileReaderException( __FILE__, __LINE__, "Both FilePattern and FilePrefix must be non-empty" );
    }

    if ( m_MatchedFiles.size() == 0 )
    {
        throw itk::ImageFileReaderException( __FILE__, __LINE__, "Sorry, there are no files to read!" );
    }


    //
    // read 3d volumes and copy them to the 4d volume
    //
    ipPicDescriptor* volume3d = NULL;
    for ( unsigned int t = 0 ; t < m_MatchedFiles.size() ; ++t )
    {
        char* filename = const_cast< char* >( m_MatchedFiles[ t ].c_str() );
        std::cout << "Reading file " << filename << "..." << std::endl;
        volume3d = ipPicGet( filename, NULL );

        if ( volume3d == NULL )
        {
            throw itk::ImageFileReaderException( __FILE__, __LINE__, "File could not be read!" );
        }

        //
        // @TODO do some error checking
        //

        //
        // @TODO left to right handed conversion
        //

        //
        // copy the 3d data volume to the 4d volume
        //
        output->SetPicVolume( volume3d, t );
        ipPicFree ( volume3d );

    }
}

mitk::PicVolumeTimeSeriesReader::PicVolumeTimeSeriesReader()
        : m_FileName( "" ), m_FilePrefix( "" ), m_FilePattern( "" )
{
    //this->DebugOn();
}

mitk::PicVolumeTimeSeriesReader::~PicVolumeTimeSeriesReader()
{}

