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

#ifndef MITK_STANDARD_FILE_LOCATIONS_H_INCLUDED_SWDG
#define MITK_STANDARD_FILE_LOCATIONS_H_INCLUDED_SWDG

#include <string>

namespace mitk
{
  /*! \brief Provides a method to look for configuration files etc.

    Call mitk::StandardFileLocations::FindFile(filename) to look for configuration files.
  */
  class StandardFileLocations
  {
    public:
      /*!
        \param filename         The file you want to fine, without any path
        \param pathInSourceDir  Where in the source tree hierarchy would that file be?
        \return The absolute path to the file including the filename

        This method looks for a file in several standard locations. The search logic is as follows

          1. If there is an environment variable MITKCONF, then try to find the file in that directory.
          2. Look in the current working directory.
          3. Use pathInSourceDir to look in a source code directory hierarchy (which is determined at compile time)
       */
      static const std::string FindFile(const char* filename, const char* pathInSourceDir = NULL );
  };

} // namespace

#endif

