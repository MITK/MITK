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

#ifndef MITK_STANDARD_FILE_LOCATIONS_H_INCLUDED_SWDG
#define MITK_STANDARD_FILE_LOCATIONS_H_INCLUDED_SWDG

#include <string>

#include <itkObject.h>
#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk
{
  /*! \brief Provides a method to look for configuration and option files etc.

  Call mitk::StandardFileLocations::FindFile(filename) to look for configuration files.
  Call mitk::StandardFileLocations::GetOptionDirectory() to look for/save option files.
  */
  class MITK_CORE_EXPORT StandardFileLocations : public itk::Object 
  {
  public:
    typedef  StandardFileLocations   Self;
    typedef  itk::Command             Superclass;
    typedef  itk::SmartPointer<Self>  Pointer;
    
    /*!
    \brief Adds a directory into the search queue:
    \      Use this function in combination with FindFile(), after adding some 
    \      directories, they will also be searched for the requested file
    \param dir         directory you want to be searched in
    \param insertInFrontOfSearchList  wheather this search request shall be processed first
    */
    void AddDirectoryForSearch(const char * dir, bool insertInFrontOfSearchList = true);
    
    /*!
    \brief Remove a directory from the search queue:
    \      Use this function in combination with FindFile().
    \      
    \param dir         directory you want to be searched in
    */
    void RemoveDirectoryForSearch(const char * dir);

    /*!
    \brief looks for a file in several standard locations
    \param filename         The file you want to fine, without any path
    \param pathInSourceDir  Where in the source tree hierarchy would that file be?
    \return The absolute path to the file including the filename

    This method appends several standard locations to the end of the searchqueue (if they not already exist)
  and then searches for the file within all directories contained in the search queue:
  
    1. Add the directory specified in the environment variable MITKCONF
  2. Add the .mitk directory in the home folder of the user
  3. Add the current working directory
    4. Add the (current working directory)/bin directory 
    5. Add the directory specified in pathInSourceDir, that is relative to the source code directory root (which is determined at compile time)
    
  Already added directories in the searchqueue by using AddDirectoryForSearch before calling FindFile are still searched first,
    because above mentioned standard locations are always appended at the end of the list.

  
  */
    std::string FindFile(const char* filename, const char* pathInSourceDir = NULL );

    /*!
    \brief Return directory of/for option files
    \return The absolute path to the directory for option files.

    This method looks for the directory of/for option files in two ways. The logic is as follows

    1. If there is an environment variable MITKOPTIONS, then use that directory.
    2. Use .mitk-subdirectory in home directory of the user

    The directory will be created if it does not exist.
    */
    std::string GetOptionDirectory();

    static StandardFileLocations* GetInstance();

  protected:

    itkNewMacro( Self );

    typedef std::vector<std::string> FileSearchVectorType;
    FileSearchVectorType  m_SearchDirectories;

    StandardFileLocations();
    virtual ~StandardFileLocations();

    std::string SearchDirectoriesForFile(const char * filename);

  private:

    // Private Copy Constructor
    StandardFileLocations( const StandardFileLocations& );
  };

} // namespace

#endif


