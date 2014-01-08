/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKIOUTIL_H
#define MITKIOUTIL_H

#include <MitkExports.h>
#include <mitkDataStorage.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkImage.h>

#include <fstream>

namespace mitk {

/**
 * \ingroup IO
 *
 * \brief A utility class to load and save data from/to the local file system.
 *
 * This method LoadFiles queries the MITK Micro Services registry for registered mitk::IDataNodeReader service
 * instances. The service instance with the highest ranking will be asked first to load the
 * given file. On error (exception thrown) or if no mitk::DataNode was constructed, the next
 * service instance is used.
 *
 * The methods LoadImage, LoadSurface, and LoadPointSet are convenience methods for general loading of
 * the three main data types in MITK. They all use the more generic method LoadDataNode.
 *
 * The methods SaveImage, SaveSurface, and SurfacePointSet are also meant for convenience (e.g. during testing).
 * Currently, there is no generic way to save a generic DataNode with an appropriate format. Thus, every method
 * initializes the corresponding instance of the special writer for the data type.
 *
 * \see mitk::IDataNodeReader
 */
class MITK_CORE_EXPORT IOUtil
{

public:

  /**
   * Get the file system path where the running executable is located.
   *
   * @return The location of the currently running executable, without the filename.
   */
  static std::string GetProgramPath();

  /**
   * Get the default temporary path.
   *
   * @return The default path for temporary data.
   */
  static std::string GetTempPath();

  /**
   * Create and open a temporary file.
   *
   * This method generates a unique temporary filename from \c templateName, creates
   * and opens the file using the output stream \c tmpStream and returns the name of
   * the newly create file.
   *
   * The \c templateName argument must contain six consective 'X' characters ("XXXXXX")
   * and these are replaced with a string that makes the filename unique.
   *
   * The file is created with read and write permissions for owner only.
   *
   * @param tmpStream The output stream for writing to the temporary file.
   * @param templateName An optional template for the filename.
   * @param path An optional path where the temporary file should be created. Defaults
   *        to the default temp path as returned by GetTempPath().
   * @return The filename of the created temporary file.
   *
   * @throw mitk::Exception if the temporary file could not be created.
   */
  static std::string CreateTemporaryFile(std::ofstream& tmpStream, const std::string& templateName = "XXXXXX", std::string path = std::string());

  /**
   * Create and open a temporary file.
   *
   * This method generates a unique temporary filename from \c templateName, creates
   * and opens the file using the output stream \c tmpStream and the specified open
   * mode \c mode and returns the name of the newly create file. The open mode is always
   * OR'd with \begin{code}std::ios_base::out | std::ios_base::trunc\end{code}.
   *
   * The \c templateName argument must contain six consective 'X' characters ("XXXXXX")
   * and these are replaced with a string that makes the filename unique.
   *
   * The file is created with read and write permissions for owner only.
   *
   * @param tmpStream The output stream for writing to the temporary file.
   * @param mode The open mode for the temporary file stream.
   * @param templateName An optional template for the filename.
   * @param path An optional path where the temporary file should be created. Defaults
   *        to the default temp path as returned by GetTempPath().
   * @return The filename of the created temporary file.
   *
   * @throw mitk::Exception if the temporary file could not be created.
   */
  static std::string CreateTemporaryFile(std::ofstream& tmpStream, std::ios_base::openmode mode,
                                         const std::string& templateName = "XXXXXX",
                                         std::string path = std::string());

  /**
   * Create a temporary directory.
   *
   * This method generates a uniquely named temporary directory from \c templateName.
   * The last set of six consecutive 'X' characters in \c templateName is replaced
   * with a string that makes the directory name unique.
   *
   * The directory is created with read, write and executable permissions for owner only.
   *
   * @param templateName An optional template for the directory name.
   * @param path An optional path where the temporary directory should be created. Defaults
   *        to the default temp path as returned by GetTempPath().
   * @return The filename of the created temporary file.
   *
   * @throw mitk::Exception if the temporary directory could not be created.
   */
  static std::string CreateTemporaryDirectory(const std::string& templateName = "XXXXXX", std::string path = std::string());

  /**
   * Load a files in <code>fileNames</code> and add the constructed mitk::DataNode instances
   * to the mitk::DataStorage <code>storage</code>
   *
   * \param fileNames A list (vector) of absolute file name paths.
   * \param storage The data storage to which the constructed data nodes are added.
   * \return The number of added mitk::DataNode instances.
   */
  static int LoadFiles(const std::vector<std::string>&fileNames, DataStorage& storage);

  /**
   * This method will create a new mitk::DataStorage instance and pass it to
   * LoadFiles(std::vector<std::string>,DataStorage).
   *
   * \param fileNames A list (vector) of absolute file name paths.
   * \return The new mitk::DataStorage containing the constructed data nodes.
   *
   * \see LoadFiles(std::vector<std::string>,DataStorage)
   */
  static DataStorage::Pointer LoadFiles(const std::vector<std::string>& fileNames);

  /**
   * @brief LoadDataNode Method to load an arbitrary DataNode.
   * @param path The path to the file including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the DataNodeFactory is not able to read/find the file
   * or the DataNode is NULL.
   * @return Returns the DataNode.
   */
  static mitk::DataNode::Pointer LoadDataNode(const std::string path);

  /**
   * @brief LoadImage Convenience method to load an arbitrary mitkImage.
   * @param path The path to the image including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the Image is NULL.
   * @return Returns the mitkImage.
   */
  static mitk::Image::Pointer LoadImage(const std::string path);

  /**
   * @brief LoadSurface Convenience method to load an arbitrary mitkSurface.
   * @param path The path to the surface including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the Surface is NULL.
   * @return Returns the mitkSurface.
   */
  static mitk::Surface::Pointer LoadSurface(const std::string path);

  /**
   * @brief LoadPointSet Convenience method to load an arbitrary mitkPointSet.
   * @param path The path to the pointset including file name and file extension (currently, only .mps is supported).
   * @throws mitk::Exception This exception is thrown when the PointSet is NULL.
   * @return Returns the mitkPointSet.
   */
  static mitk::PointSet::Pointer LoadPointSet(const std::string path);

  /**
   * @brief SaveImage Convenience method to save an arbitrary mitkImage.
   * @param path The path to the image including file name and file extension.
   * If not extention is set, the default value (defined in DEFAULTIMAGEEXTENSION) is used.
   * @param image The image to save.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the image.
   * @return Returns true for success else false.
   */
  static bool SaveImage(mitk::Image::Pointer image, const std::string path);

  /**
   * @brief SaveBaseData Convenience method to save arbitrary baseData.
   * @param path The path to the image including file name and file extension.
   * If not extention is set, the default value (defined in DEFAULTIMAGEEXTENSION) is used.
   * @param data The data to save.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the image.
   * @return Returns true for success else false.
   */
  static bool SaveBaseData(mitk::BaseData* data, const std::string& path);

  /**
   * @brief SaveSurface Convenience method to save an arbitrary mitkSurface.
   * @param path The path to the surface including file name and file extension.
   * If not extention is set, the default value (defined in DEFAULTSURFACEEXTENSION) is used.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the surface.
   * or if the fileextension is not suitable for writing.
   * @return Returns true for success else false.
   */
  static bool SaveSurface(mitk::Surface::Pointer surface, const std::string path);

  /**
   * @brief SavePointSet Convenience method to save an mitkPointSet.
   * @param path The path to the pointset including file name and file extension (currently, only .mps is supported).
   * If not extention is set, the default value (defined in DEFAULTPOINTSETEXTENSION) is used.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the pointset.
   * @return Returns true for success else false.
   */
  static bool SavePointSet(mitk::PointSet::Pointer pointset, const std::string path);

  static const std::string DEFAULTIMAGEEXTENSION;
  static const std::string DEFAULTSURFACEEXTENSION;
  static const std::string DEFAULTPOINTSETEXTENSION;

};

}

#endif // MITKIOUTIL_H
