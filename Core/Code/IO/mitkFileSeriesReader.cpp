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


#include "mitkFileSeriesReader.h"
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <map>



bool mitk::FileSeriesReader::GenerateFileList()
{
    typedef std::vector<std::string> StringContainer;
    typedef std::map<unsigned int, std::string> SortedStringContainer;
    
    if ( m_FileName == "" )
    {
      throw itk::ImageFileReaderException( __FILE__, __LINE__, "FileName must be non-empty" );
    }
    //MITK_INFO << "FileName: "<< m_FileName <<", FilePrefix: "<< m_FilePrefix << ", FilePattern: "<< m_FilePattern << std::endl;
    
    // determine begin and end idexes of the last digit sequence in the 
    // filename from the sample file name
    // Therefore, walk backwards from the end of the filename until
    // a number is found. The string in front of the number is the prefix,
    // the string after the number is the extension.
    std::string basename, path;
    path = itksys::SystemTools::GetFilenamePath( m_FileName );
    basename = itksys::SystemTools::GetFilenameName( m_FileName );
    
    unsigned int digitBegin = 0;
    unsigned int digitEnd = 0;
    bool digitFound = false;
    for ( unsigned int i = basename.length() - 1; ; --i )
    {
      char character = basename[ i ];
      if ( character >= '0' && character <= '9' )
      {
        if (!digitFound)
        {
          digitEnd = i;
          digitBegin = i;  
          digitFound = true;
        } 
        else
          digitBegin = i;
      }
      else
      {
        //end of digit series found, jump out of loop!
        if (digitFound)
          break;
      }
      if ( i == 0 )
        break;
    }
    
    //
    // if there is no digit in the filename, then we have a problem
    // no matching filenames can be identified!
    // 
    if ( !digitFound )
    {
      itkWarningMacro("Filename contains no digit!");
      return false;
    }
    
    //
    // determine prefix and extension start and length
    //
    unsigned int prefixBegin = 0;
    unsigned int prefixLength = digitBegin;
    unsigned int extensionBegin = digitEnd + 1;
    unsigned int extensionLength = (digitEnd == basename.length() -1 ? 0 : basename.length() - 1 - digitEnd);    
    unsigned int numberLength = digitEnd - digitBegin + 1;
    
    //
    // extract prefix and extension
    //
    std::string prefix = "";
    if (prefixLength != 0)
      prefix = basename.substr( prefixBegin, prefixLength );
    std::string extension = "";
    if (extensionLength != 0)
      extension = basename.substr( extensionBegin, extensionLength );
    
    //
    // print debug information
    //
    /*
    MITK_INFO << "digitBegin      : " << digitBegin << std::endl;
    MITK_INFO << "digitEnd        : " << digitEnd << std::endl;
    MITK_INFO << "number of digits: " << numberLength << std::endl;
    MITK_INFO << "prefixBegin     : " << prefixBegin << std::endl;
    MITK_INFO << "prefixLength    : " << prefixLength << std::endl;
    MITK_INFO << "prefix          : " << prefix << std::endl;
    MITK_INFO << "extensionBegin  : " << extensionBegin << std::endl;
    MITK_INFO << "extensionLength : " << extensionLength << std::endl;
    MITK_INFO << "extension       : " << extension << std::endl;
    */
    if( (prefixLength + extensionLength + numberLength) != basename.length() )
    {
      throw itk::ImageFileReaderException( __FILE__, __LINE__, "prefixLength + extensionLength + numberLength != basenameLength" );
    }
    
    
    //
    // Load Directory
    //
    std::string directory = itksys::SystemTools::GetFilenamePath( m_FileName );
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
    //unsigned long i;
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
    for ( StringContainer::iterator it = unmatchedFiles.begin() ; it != unmatchedFiles.end() ; ++it )
    {
        bool prefixMatch = false;
        bool extensionMatch = false;
        
        // check if the file prefix matches the current file
        if ( prefixLength != 0 )
          prefixMatch = ( it->find(prefix) == prefixBegin ); // check if prefix is found
        else
          prefixMatch = ( ( (*it)[0] >='0' ) && ( (*it)[0] <='9' ) ); //check if filename begins with digit
        
        // check if the file extension matches the current file
        if ( extensionLength != 0 )
          extensionMatch = ( it->find(extension) == it->length() - extensionLength ); // check if prefix is found
        else
          extensionMatch = ( ( (*it)[it->length()-1] >='0' ) && ( (*it)[it->length()-1] <='9' ) ); //check if filename ends with digit
        
        if ( prefixMatch && extensionMatch )
        {
            matchedFiles.push_back( *it );
        }
    }
    if ( matchedFiles.size() == 0 )
    {
      itkWarningMacro( << "Sorry, none of the files matched the prefix!" );
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
      // parse the filename starting from pos digitBegin until we reach a non-digit
      // or the end of filename
      std::string number = "";
      std::string currentFilename(*it);
      for ( unsigned int i = digitBegin ; i < currentFilename.length() ; ++i)
      {
        char character = currentFilename[ i ];
        //do we have a digit?
        if ( character >= '0' && character <= '9' )
          number += character;
        else
          break; //end of digit series found, jump out of loop!
      }
      if ( number.length() == 0 )
      {
        // The file is not numbered, this is an error!
        // Nevertheless, we try the next files.
        itkWarningMacro( << "The filename " << *it << "does not contain a valid digit sequence but matches prefix and extension. Skipping file!" );
      }
      else
      {
        if ( ( number.length() + prefix.length() + extension.length() ) != it->length() )
        {
          itkWarningMacro("The file "<< *it <<" matches prefix and extension, but the string in beteen is not a single digit-sequence. Skipping file!");
        }
        else
        {
        // convert the number string into an integer and
        // insert the filname (including directory) into the SortedStringContainer
        unsigned int num = atoi( number.c_str() );
        sortedFiles.insert( std::make_pair( num, directory + "/" + *it ) );
        }
      }
    }
    if ( sortedFiles.size() == 0 )
    {
      itkWarningMacro( << "Sorry, no numbered files found, I can't load anything..." );
      return false;
    }

    //
    // Convert the sorted string container in a plain sorted vector of strings;
    //
    m_MatchedFileNames.clear();
    m_MatchedFileNames.resize( sortedFiles.size() );
    unsigned long index = 0;
    for ( SortedStringContainer::iterator it = sortedFiles.begin() ; it != sortedFiles.end() ; ++it, ++index )
    {
        m_MatchedFileNames[ index ] = it->second ;
        MITK_INFO << "Added " << it->second << " to the set of matched files!" << std::endl;
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
