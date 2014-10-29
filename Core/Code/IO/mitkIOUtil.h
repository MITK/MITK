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

#include <MitkCoreExports.h>
#include <mitkDataStorage.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkImage.h>

#include <mitkIFileReader.h>
#include <mitkIFileWriter.h>
#include <mitkFileReaderSelector.h>
#include <mitkFileWriterSelector.h>

#include <fstream>

namespace mitk {

/**
 * \ingroup IO
 *
 * \brief A utility class to load and save data from/to the local file system.
 *
 * The methods LoadImage, LoadSurface, and LoadPointSet are convenience methods for general loading of
 * the three main data types in MITK. They all use the more generic method Load().
 *
 * \see QmitkIOUtil
 */
class MITK_CORE_EXPORT IOUtil
{

public:

  struct MITK_CORE_EXPORT LoadInfo
  {
    LoadInfo(const std::string& path);

    std::string m_Path;
    std::vector<BaseData::Pointer> m_Output;

    FileReaderSelector m_ReaderSelector;
    bool m_Cancel;
  };

  struct MITK_CORE_EXPORT SaveInfo
  {
    SaveInfo(const BaseData* baseData, const MimeType& mimeType, const std::string& path);

    bool operator<(const SaveInfo& other) const;

    /// The BaseData object to save.
    const BaseData* m_BaseData;

    /// Contains a set of IFileWriter objects.
    FileWriterSelector m_WriterSelector;
    /// The selected mime-type, used to restrict results from FileWriterSelector.
    MimeType m_MimeType;
    /// The path to write the BaseData object to.
    std::string m_Path;
    /// Flag indicating if sub-sequent save operations are to be canceled.
    bool m_Cancel;
  };

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
  * Returns the Directory Seperator for the current OS.
  *
  * @return the Directory Seperator for the current OS, i.e. "\\" for Windows and "/" otherwise.
  */
  static char GetDirectorySeparator();

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
   * Creates an empty temporary file.
   *
   * This method generates a unique temporary filename from \c templateName and creates
   * this file.
   *
   * The file is created with read and write permissions for owner only.
   *
   * ---
   * This version is potentially unsafe because the created temporary file is not kept open
   * and could be used by another process between calling this method and opening the returned
   * file path for reading or writing.
   * ---
   *
   * @return The filename of the created temporary file.
   * @param templateName An optional template for the filename.
   * @param path An optional path where the temporary file should be created. Defaults
   *        to the default temp path as returned by GetTempPath().
   * @throw mitk::Exception if the temporary file could not be created.
   */
  static std::string CreateTemporaryFile(const std::string& templateName = "XXXXXX",
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
   * @brief Load a file into the given DataStorage.
   *
   * This method calls Load(const std::vector<std::string>&, DataStorage&) with a
   * one-element vector.
   *
   * @param path The absolute file name including the file extension.
   * @param storage A DataStorage object to which the loaded data will be added.
   * @return The set of added DataNode objects.
   * @throws mitk::Exception if \c path could not be loaded.
   *
   * @sa Load(const std::vector<std::string>&, DataStorage&)
   */
  static DataStorage::SetOfObjects::Pointer Load(const std::string& path, DataStorage& storage);

  static DataStorage::SetOfObjects::Pointer Load(const std::string& path,
                                                 const IFileReader::Options& options,
                                                 DataStorage& storage);

  static std::vector<BaseData::Pointer> Load(const std::string& path);

  static std::vector<BaseData::Pointer> Load(const std::string& path,
                                             const IFileReader::Options& options);

  /**
   * @brief Loads a list of file paths into the given DataStorage.
   *
   * If an entry in \c paths cannot be loaded, this method will continue to load
   * the remaining entries into \c storage and throw an exception afterwards.
   *
   * @param paths A list of absolute file names including the file extension.
   * @param storage A DataStorage object to which the loaded data will be added.
   * @return The set of added DataNode objects.
   * @throws mitk::Exception if an entry in \c paths could not be loaded.
   */
  static DataStorage::SetOfObjects::Pointer Load(const std::vector<std::string>& paths, DataStorage& storage);

  static std::vector<BaseData::Pointer> Load(const std::vector<std::string>& paths);

  /**
   * Load files in <code>fileNames</code> and add the constructed mitk::DataNode instances
   * to the mitk::DataStorage <code>storage</code>
   *
   * @param fileNames A list (vector) of absolute file name paths.
   * @param storage The data storage to which the constructed data nodes are added.
   * @return The number of added mitk::DataNode instances.
   *
   * @deprecatedSince{2014_10} Use Load() instead
   */
  DEPRECATED(static int LoadFiles(const std::vector<std::string>&fileNames, DataStorage& storage));

  /**
   * This method will create a new mitk::DataStorage instance and pass it to
   * LoadFiles(std::vector<std::string>,DataStorage).
   *
   * @param fileNames A list (vector) of absolute file name paths.
   * @return The new mitk::DataStorage containing the constructed data nodes.
   *
   * @see LoadFiles(std::vector<std::string>,DataStorage)
   *
   * @deprecatedSince{2014_10} Use Load() instead
   */
  DEPRECATED(static DataStorage::Pointer LoadFiles(const std::vector<std::string>& fileNames));

  /**
   * @brief Create a BaseData object from the given file.
   * @param path The path to the file including file name and file extension.
   * @throws mitk::Exception In case of an error when reading the file.
   * @return Returns the created BaseData object.
   *
   * @deprecatedSince{2014_10} Use Load() or FileReaderRegistry::Read() instead.
   */
  DEPRECATED(static mitk::BaseData::Pointer LoadBaseData(const std::string& path));

  /**
   * @brief LoadDataNode Method to load an arbitrary DataNode.
   * @param path The path to the file including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the DataNodeFactory is not able to read/find the file
   * or the DataNode is NULL.
   * @return Returns the DataNode.
   *
   * @deprecatedSince{2014_10} Use Load() instead.
   */
  DEPRECATED(static mitk::DataNode::Pointer LoadDataNode(const std::string& path));

  /**
   * @brief LoadImage Convenience method to load an arbitrary mitkImage.
   * @param path The path to the image including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the Image is NULL.
   * @return Returns the mitkImage.
   */
  static mitk::Image::Pointer LoadImage(const std::string& path);

  /**
   * @brief LoadSurface Convenience method to load an arbitrary mitkSurface.
   * @param path The path to the surface including file name and file extension.
   * @throws mitk::Exception This exception is thrown when the Surface is NULL.
   * @return Returns the mitkSurface.
   */
  static mitk::Surface::Pointer LoadSurface(const std::string& path);

  /**
   * @brief LoadPointSet Convenience method to load an arbitrary mitkPointSet.
   * @param path The path to the pointset including file name and file extension (currently, only .mps is supported).
   * @throws mitk::Exception This exception is thrown when the PointSet is NULL.
   * @return Returns the mitkPointSet.
   */
  static mitk::PointSet::Pointer LoadPointSet(const std::string& path);


  /**
   * @brief Save a mitk::BaseData instance.
   * @param data The data to save.
   * @param path The path to the image including file name and and optional file extension.
   *        If no extension is set, the default extension and mime-type for the
   *        BaseData type of \c data is used.
   * @throws mitk::Exception if no writer for \c data is available or the writer
   *         is not able to write the image.
   */
  static void Save(const mitk::BaseData* data, const std::string& path);

  /**
   * @brief Save a mitk::BaseData instance.
   * @param data The data to save.
   * @param path The path to the image including file name and an optional file extension.
   *        If no extension is set, the default extension and mime-type for the
   *        BaseData type of \c data is used.
   * @param options The IFileWriter options to use for the selected writer.
   * @throws mitk::Exception if no writer for \c data is available or the writer
   *         is not able to write the image.
   */
  static void Save(const mitk::BaseData* data, const std::string& path, const IFileWriter::Options& options);

  /**
   * @brief Save a mitk::BaseData instance.
   * @param data The data to save.
   * @param mimeType The mime-type to use for writing \c data.
   * @param path The path to the image including file name and an optional file extension.
   * @param addExtension If \c true, an extension according to the given \c mimeType
   *        is added to \c path if it does not contain one. If \c path already contains
   *        a file name extension, it is not checked for compatibility with \c mimeType.
   *
   * @throws mitk::Exception if no writer for the combination of \c data and \c mimeType is
   *         available or the writer is not able to write the image.
   */
  static void Save(const mitk::BaseData* data, const std::string& mimeType, const std::string& path,
                   bool addExtension = true);

  /**
   * @brief Save a mitk::BaseData instance.
   * @param data The data to save.
   * @param mimeType The mime-type to use for writing \c data.
   * @param path The path to the image including file name and an optional file extension.
   * @param options Configuration data for the used IFileWriter instance.
   * @param addExtension If \c true, an extension according to the given \c mimeType
   *        is added to \c path if it does not contain one. If \c path already contains
   *        a file name extension, it is not checked for compatibility with \c mimeType.
   *
   * @throws mitk::Exception if no writer for the combination of \c data and \c mimeType is
   *         available or the writer is not able to write the image.
   */
  static void Save(const mitk::BaseData* data, const std::string& mimeType, const std::string& path,
                   const mitk::IFileWriter::Options& options, bool addExtension = true);

  /**
   * @brief Use SaveInfo objects to save BaseData instances.
   *
   * This is a low-level method for directly working with SaveInfo objects. Usually,
   * the Save() methods taking a BaseData object as an argument are more appropriate.
   *
   * @param saveInfos A list of SaveInfo objects for saving contained BaseData objects.
   *
   * @see Save(const mitk::BaseData*, const std::string&)
   */
  static void Save(std::vector<SaveInfo>& saveInfos);

  /**
   * @brief SaveImage Convenience method to save an arbitrary mitkImage.
   * @param path The path to the image including file name and file extension.
   *        If no extention is set, the default value (defined in DEFAULTIMAGEEXTENSION) is used.
   * @param image The image to save.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the image.
   * @return Returns true for success else false.
   *
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static bool SaveImage(mitk::Image::Pointer image, const std::string& path));

  /**
   * @brief SaveBaseData Convenience method to save arbitrary baseData.
   * @param path The path to the image including file name and file extension.
   *        If no extention is set, the default extension is used.
   * @param data The data to save.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the image.
   * @return Returns true for success else false.
   *
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static bool SaveBaseData(mitk::BaseData* data, const std::string& path));

  /**
   * @brief SaveSurface Convenience method to save an arbitrary mitkSurface.
   * @param path The path to the surface including file name and file extension.
   *        If no extention is set, the default extension is used.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the surface.
   * or if the fileextension is not suitable for writing.
   * @return Returns true for success else false.
   *
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static bool SaveSurface(mitk::Surface::Pointer surface, const std::string& path));

  /**
   * @brief SavePointSet Convenience method to save an mitkPointSet.
   * @param path The path to the pointset including file name and file extension.
   *        If no extention is set, the default extension is used.
   * @throws mitk::Exception This exception is thrown when the writer is not able to write the pointset.
   * @return Returns true for success else false.
   *
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static bool SavePointSet(mitk::PointSet::Pointer pointset, const std::string& path));

  /** @deprecatedSince{2014_10} */
  DEPRECATED(static const std::string DEFAULTIMAGEEXTENSION);
  /** @deprecatedSince{2014_10} */
  DEPRECATED(static const std::string DEFAULTSURFACEEXTENSION);
  /** @deprecatedSince{2014_10} */
  DEPRECATED(static const std::string DEFAULTPOINTSETEXTENSION);

protected:

  struct ReaderOptionsFunctorBase
  {
    virtual bool operator()(LoadInfo& loadInfo) = 0;
  };

  struct WriterOptionsFunctorBase
  {
    virtual bool operator()(SaveInfo& saveInfo) = 0;
  };

  static std::string Load(std::vector<LoadInfo>& loadInfos, DataStorage::SetOfObjects* nodeResult,
                          DataStorage* ds, ReaderOptionsFunctorBase* optionsCallback);

  static std::string Save(const BaseData* data, const std::string& mimeType, const std::string& path,
                          WriterOptionsFunctorBase* optionsCallback, bool addExtension);

  static std::string Save(std::vector<SaveInfo>& saveInfos,
                          WriterOptionsFunctorBase* optionsCallback);

private:

  struct Impl;

};

}

#endif // MITKIOUTIL_H
