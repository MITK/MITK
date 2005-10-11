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


#include "mitkFileSeriesReader.h"
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <map>

bool mitk::FileSeriesReader::GenerateFileList()
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
    unsigned long i;
    for ( i = 0; i < itkDir.GetNumberOfFiles(); i++ )
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
        return false;
    }

    //
    // Convert the sorted string container in a plain sorted vector of strings;
    //
    m_MatchedFileNames.clear();
    m_MatchedFileNames.resize( sortedFiles.size() );
    i = 0;
    for ( SortedStringContainer::iterator it = sortedFiles.begin() ; it != sortedFiles.end() ; ++it, ++i )
    {
        m_MatchedFileNames[ i ] = it->second ;
        //std::cout << "Added " << it->second << " to the set of matched files!" << std::endl;
    }
    return true;
}


mitk::FileSeriesReader::MatchedFileNames mitk::FileSeriesReader::GetMatchedFileNames( )
{
    return m_MatchedFileNames;
}

mitk::FileSeriesReader::FileSeriesReader()
   : m_FileName( "" ), m_FilePrefix( "" ), m_FilePattern( "" )
{}

mitk::FileSeriesReader::~FileSeriesReader()
{
}
