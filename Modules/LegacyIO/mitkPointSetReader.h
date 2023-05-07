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
   * @brief reads xml representations of mitk::PointSets from a file
   *
   * Reader for xml files containing one or multiple xml represenations of
   * mitk::PointSets. If multiple mitk::PointSets are stored in one file,
   * these are assigned to multiple outputs of the filter. The number of point
   * sets which have be read can be retrieven by a call to GetNumberOfOutputs()
   * after the pipeline update().
   * The reader is able to read the old 3D Pointsets without the "specification" and "timeseries" tags and the new 4D
   * Pointsets.
   * @note loading point sets from multiple files according to a given file pattern
   * is not yet supported!
   *
   * @ingroup MitkLegacyIOModule
   *
   * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
  */
  class MITKLEGACYIO_EXPORT PointSetReader : public PointSetSource, public FileReader
  {
  public:
    mitkClassMacro(PointSetReader, FileReader);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * @brief Sets the filename of the file to be read
       * @param _arg the filename of the point set xml-file
       */
      itkSetStringMacro(FileName);

    /**
     * @brief Returns the filename of the point set xml-file.
     * @returns the filename of the point set xml-file.
     */
    itkGetStringMacro(FileName);

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro(FilePrefix);

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro(FilePrefix);

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro(FilePattern);

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

    /**
     * @returns whether the last read attempt was successful or not.
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
     * @param num the new number of output objects.
     */
    virtual void ResizeOutputs(const unsigned int &num);

    /**
     * Checks if the given file has appropriate
     * read access.
     * @returns true if the file exists and may be read
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
