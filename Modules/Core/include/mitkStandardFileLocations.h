/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStandardFileLocations_h
#define mitkStandardFileLocations_h

#include <string>

#include <MitkCoreExports.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{
  /**
   * \brief Provides methods to look for configuration and option files in standard locations.
   *
   * Call mitk::StandardFileLocations::FindFile(filename) to look for configuration files.
   * Call mitk::StandardFileLocations::GetOptionDirectory() to look for/save option files.
   */
  class MITKCORE_EXPORT StandardFileLocations : public itk::Object
  {
  public:
    typedef StandardFileLocations Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;

    /**
     * \brief Add a directory to the search queue.
     *
     * Use this function in combination with FindFile(). After adding some
     * directories, they will also be searched for the requested file.
     *
     * \param[in] dir directory you want to be searched in.
     * \param[in] insertInFrontOfSearchList whether this search request shall be processed first.
     */
    void AddDirectoryForSearch(const char *dir, bool insertInFrontOfSearchList = true);

    /**
     * \brief Remove a directory from the search queue.
     *
     * Use this function in combination with FindFile().
     *
     * \param[in] dir directory to remove from the search queue.
     */
    void RemoveDirectoryForSearch(const char *dir);

    /**
     * \brief Look for a file in several standard locations.
     *
     * This method appends several standard locations to the end of the search queue
     * (if they do not already exist) and then searches for the file within all
     * directories contained in the search queue:
     *
     * <ol>
     *   <li> Add the directory specified in the environment variable MITKCONF
     *   <li> Add the .mitk directory in the home folder of the user
     *   <li> Add the current working directory
     *   <li> Add the (current working directory)/bin directory
     *   <li> Add the directory specified in pathInSourceDir, relative to the source
     *        code directory root (determined at compile time)
     * </ol>
     *
     * Directories already added via AddDirectoryForSearch() before calling FindFile()
     * are still searched first, because above mentioned standard locations are always
     * appended at the end of the list.
     *
     * \param[in] filename the file you want to find, without any path.
     * \param[in] pathInSourceDir where in the source tree hierarchy the file would be.
     * \return The absolute path to the file including the filename, or an empty string
     *         if the file was not found.
     */
    std::string FindFile(const char *filename, const char *pathInSourceDir = nullptr);

    /**
     * \brief Return the directory for option files.
     *
     * This method looks for the directory for option files in two ways:
     *
     * 1. If there is an environment variable MITKOPTIONS, then use that directory.
     * 2. Use .mitk-subdirectory in the home directory of the user.
     *
     * The directory will be created if it does not exist.
     *
     * \return The absolute path to the directory for option files.
     */
    std::string GetOptionDirectory();

    /**
     * \brief Get the singleton instance of StandardFileLocations.
     * \return Pointer to the singleton instance.
     */
    static StandardFileLocations *GetInstance();

  protected:
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef std::vector<std::string> FileSearchVectorType;
    FileSearchVectorType m_SearchDirectories;

    StandardFileLocations();
    ~StandardFileLocations() override;

    /**
     * \brief Search all directories in the search queue for the given file.
     *
     * \param[in] filename the file to search for.
     * \return The absolute path to the file, or an empty string if not found.
     */
    std::string SearchDirectoriesForFile(const char *filename);

  private:
    // Private Copy Constructor
    StandardFileLocations(const StandardFileLocations &);
  };

} // namespace

#endif
