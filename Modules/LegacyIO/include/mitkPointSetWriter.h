/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetWriter_h
#define mitkPointSetWriter_h

#include <MitkLegacyIOExports.h>

#include <itkProcessObject.h>
#include <mitkFileWriter.h>
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * \brief XML-based writer for mitk::PointSet objects.
   *
   * Writes one or more mitk::PointSet objects to a single XML file. Multiple
   * point sets can be written by setting multiple inputs to this filter via
   * SetInput(num, input). The file format uses MITK's custom XML schema with
   * elements for point set files, time series, and individual points with
   * their coordinates and identifiers.
   *
   * \note Writing of multiple XML files according to a filename pattern is not
   *       yet supported.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   * \sa mitk::PointSetReader, mitk::PointSetWriterFactory, mitk::FileWriter
   */
  class MITKLEGACYIO_EXPORT PointSetWriter : public mitk::FileWriter
  {
  public:
    mitkClassMacro(PointSetWriter, mitk::FileWriter);

    mitkWriterMacro;

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief The input data type for this writer. */
    typedef mitk::PointSet InputType;

    /** \brief Smart pointer type for the input data type. */
    typedef InputType::Pointer InputTypePointer;

    /**
     * \brief Set the filename of the file to write.
     */
    itkSetStringMacro(FileName);

    /**
     * \brief Get the filename of the file to be written.
     * \return The output file name.
     */
    itkGetStringMacro(FileName);

    /**
     * \brief Set the file prefix for multi-file writing.
     * \warning Multiple file writing is not yet supported.
     */
    itkSetStringMacro(FilePrefix);

    /**
     * \brief Get the file prefix.
     * \warning Multiple file writing is not yet supported.
     */
    itkGetStringMacro(FilePrefix);

    /**
     * \brief Set the file pattern for multi-file writing.
     * \warning Multiple file writing is not yet supported.
     */
    itkSetStringMacro(FilePattern);

    /**
     * \brief Get the file pattern.
     * \warning Multiple file writing is not yet supported.
     */
    itkGetStringMacro(FilePattern);

    /**
     * \brief Set the 0th input object for the filter.
     * \param[in] input The first point set to write.
     */
    void SetInput(InputType *input);

    /**
     * \brief Set the n-th input object for the filter.
     *
     * If \p num is larger than GetNumberOfInputs(), the number of inputs is
     * resized appropriately.
     *
     * \param[in] num The zero-based index of the input to set.
     * \param[in] input The point set to assign to input slot \p num.
     */
    void SetInput(const unsigned int &num, InputType *input);

    /**
     * \brief Get the 0th input point set.
     * \return Pointer to the first input point set, or \c nullptr if not set.
     */
    PointSet *GetInput();

    /**
     * \brief Get the n-th input point set.
     * \param[in] num The zero-based index of the desired input.
     * \return Pointer to the n-th input point set, or \c nullptr if not set.
     */
    PointSet *GetInput(const unsigned int &num);

    /**
     * \brief Return the list of possible file extensions for point set data.
     * \return A vector of supported file extension strings (e.g., ".mps").
     */
    std::vector<std::string> GetPossibleFileExtensions() override;

    /**
     * \brief Return the class name of the supported base data type.
     * \return The static class name of mitk::PointSet.
     */
    std::string GetSupportedBaseData() const override;

    /**
     * \brief Return the file extension to be added to the filename.
     * \return The file extension string.
     */
    std::string GetFileExtension() override;

    /**
     * \brief Check whether the writer can write data from the given DataNode.
     * \param[in] node The DataNode to check. The node's data must be a mitk::PointSet.
     * \return \c true if the node contains a mitk::PointSet; \c false otherwise.
     */
    bool CanWriteDataType(DataNode * node) override;

    /**
     * \brief Return the MIME type of the file to be written.
     * \return The MIME type string.
     */
    std::string GetWritenMIMEType() override;

    using mitk::FileWriter::SetInput;

    /**
     * \brief Set a DataNode as input. Extracts the mitk::PointSet from the node.
     * \param[in] node The DataNode containing the point set to write.
     */
    virtual void SetInput(DataNode * node);

    /**
     * \brief Query whether the last write attempt was successful.
     * \return \c true if the last call to Update() succeeded; \c false otherwise.
     */
    bool GetSuccess() const;

  protected:
    /**
     * Constructor.
     */
    PointSetWriter();

    /**
     * Virtual destructor.
     */
    ~PointSetWriter() override;

    /**
     * Writes the XML file
     */
    void GenerateData() override;

    /**
     * Resizes the number of inputs of the writer.
     * The inputs are initialized by empty PointSets
     * \param num the new number of inputs
     */
    virtual void ResizeInputs(const unsigned int &num);

    /**
     * Converts an arbitrary type to a string. The type has to
     * support the << operator. This works fine at least for integral
     * data types as float, int, long etc.
     * \param value the value to convert
     * \return the string representation of value
     */
    template <typename T>
    std::string ConvertToString(T value);

    /**
     * Writes an XML representation of the given point set to
     * an outstream. The XML-Header an root node is not included!
     * \param pointSet the point set to be converted to xml
     * \param out the stream to write to.
     */
    void WriteXML(mitk::PointSet *pointSet, std::ofstream &out);

    /**
     * Writes an standard xml header to the given stream.
     * \param file the stream in which the header is written.
     */
    void WriteXMLHeader(std::ofstream &file);

    /** Write a start element tag */
    void WriteStartElement(const char *const tag, std::ofstream &file);

    /**
     * Write an end element tag
     * End-Elements following character data should pass indent = false.
     */
    void WriteEndElement(const char *const tag, std::ofstream &file, const bool &indent = true);

    /** Write character data inside a tag. */
    void WriteCharacterData(const char *const data, std::ofstream &file);

    /** Write a start element tag */
    void WriteStartElement(std::string &tag, std::ofstream &file);

    /** Write an end element tag */
    void WriteEndElement(std::string &tag, std::ofstream &file, const bool &indent = true);

    /** Write character data inside a tag. */
    void WriteCharacterData(std::string &data, std::ofstream &file);

    /** Writes empty spaces to the stream according to m_IndentDepth and m_Indent */
    void WriteIndent(std::ofstream &file);

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    unsigned int m_IndentDepth;

    unsigned int m_Indent;

    bool m_Success;

  public:
    /** \name XML Tag Name Constants
     *  String constants for the XML element and attribute names used in the
     *  point set file format.
     */
    //\{
    static const char *XML_POINT_SET;      ///< \brief XML element name for a single point set ("point_set").
    static const char *XML_TIME_SERIES;    ///< \brief XML element name for a time series ("time_series").
    static const char *XML_TIME_SERIES_ID; ///< \brief XML element name for a time series identifier ("time_series_id").
    static const char *XML_POINT_SET_FILE; ///< \brief XML root element name ("point_set_file").
    static const char *XML_FILE_VERSION;   ///< \brief XML element name for the file version ("file_version").
    static const char *XML_POINT;          ///< \brief XML element name for an individual point ("point").
    static const char *XML_SPEC;           ///< \brief XML element name for point specification ("specification").
    static const char *XML_ID;             ///< \brief XML element name for a point identifier ("id").
    static const char *XML_X;              ///< \brief XML element name for the X coordinate ("x").
    static const char *XML_Y;              ///< \brief XML element name for the Y coordinate ("y").
    static const char *XML_Z;              ///< \brief XML element name for the Z coordinate ("z").
    static const char *VERSION_STRING;     ///< \brief The current file format version string.
    //\}
  };
}

#endif
