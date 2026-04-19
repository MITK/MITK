/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetReader_h
#define mitkPointSetReader_h

#include <MitkLegacyIOExports.h>

#include <mitkFileReader.h>
#include <mitkPointSetSource.h>
#include <stack>
#include <string>
#include <vtkXMLParser.h>

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  /**
   * \brief XML-based reader for mitk::PointSet files.
   *
   * Reads XML files containing one or more serialized mitk::PointSet objects.
   * If multiple point sets are stored in a single file, each is assigned to a
   * separate output of this filter. The number of loaded point sets can be
   * queried via GetNumberOfOutputs() after calling Update().
   *
   * The reader supports both the legacy 3D point set format (without "specification"
   * and "timeseries" tags) and the newer 4D point set format with time series support.
   *
   * \note Loading point sets from multiple files according to a file pattern
   *       is not yet supported.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::PointSetWriter, mitk::PointSetIOFactory, mitk::PointSetSource
   */
  class MITKLEGACYIO_EXPORT PointSetReader : public PointSetSource, public FileReader
  {
  public:
    mitkClassMacro(PointSetReader, FileReader);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Set the absolute path to the point set XML file to read. */
    mitkOverrideSetStringMacro(FileName);

    /**
     * \brief Get the filename of the point set XML file.
     * \return The file path that was set via SetFileName().
     */
    mitkOverrideGetStringMacro(FileName);

    /**
     * \brief Set the file prefix for multi-file loading.
     * \warning Multiple file loading is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePrefix);

    /**
     * \brief Get the file prefix.
     * \warning Multiple file loading is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePrefix);

    /**
     * \brief Set the file pattern for multi-file loading.
     * \warning Multiple file loading is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePattern);

    /**
     * \brief Get the file pattern.
     * \warning Multiple file loading is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Check whether the given file can be read by this reader.
     * \param[in] filename The file path to check.
     * \param[in] filePrefix The file prefix (currently unused).
     * \param[in] filePattern The file pattern (currently unused).
     * \return \c true if the file can be read (checks for ".mps" extension);
     *         \c false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

    /**
     * \brief Query whether the last read attempt was successful.
     * \return \c true if the last call to Update() successfully read point set data;
     *         \c false otherwise.
     */
    bool GetSuccess() const;

  protected:
    /**
     * Constructor
     */
    PointSetReader();

    /**
     * Virtual destructor
     */
    ~PointSetReader() override;

    /**
     * Actually reads the point sets from the given file
     */
    void GenerateData() override;

    virtual mitk::PointSet::Pointer ReadPoint(mitk::PointSet::Pointer newPointSet,
                                              const tinyxml2::XMLElement *currentTimeSeries,
                                              unsigned int currentTimeStep);

    /**
     * Does nothing in the current implementation
     */
    void GenerateOutputInformation() override;

    /**
     * Resizes the output-objects according to the given number.
     * \param num the new number of output objects.
     */
    virtual void ResizeOutputs(const unsigned int &num);

    /**
     * Checks if the given file has appropriate
     * read access.
     * \return true if the file exists and may be read
     *          or false otherwise.
     */
    virtual int CanReadFile(const char *name);

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;
  };
}

#endif
