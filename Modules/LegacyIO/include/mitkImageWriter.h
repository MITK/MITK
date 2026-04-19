/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageWriter_h
#define mitkImageWriter_h

#include <MitkLegacyIOExports.h>
#include <mitkFileWriterWithInformation.h>

namespace mitk
{
  class Image;

  /**
   * \brief Writer for mitk::Image objects to various file formats.
   *
   * Uses the configured file extension (SetExtension()) to decide the output format.
   * The default extension is ".mhd" (MetaImage). Other supported formats include
   * ".tif", ".png", and ".jpg". The writer delegates to ITK image writers internally
   * and supports optional compression.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::FileWriterWithInformation, mitk::IOUtil
   */
  class MITKLEGACYIO_EXPORT ImageWriter : public mitk::FileWriterWithInformation
  {
  public:
    mitkClassMacro(ImageWriter, mitk::FileWriter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkWriterMacro;

    /**
     * \brief Set the filename of the file to write.
     * \param[in] fileName The name of the file to write (C-string variant).
     */
    void SetFileName(const char *fileName) override;

    /**
     * \brief Set the filename of the file to write.
     * \param[in] fileName The name of the file to write (std::string variant).
     */
    virtual void SetFileName(const std::string &fileName);

    /**
     * \brief Get the filename of the file to be written.
     * \return The output file name.
     */
    mitkOverrideGetStringMacro(FileName);

    /**
     * \brief Explicitly set the extension to be added to the filename.
     * \param[in] extension Extension to be added to the filename, including a "."
     *            (e.g., ".mhd").
     */
    virtual void SetExtension(const char *extension);

    /**
     * \brief Explicitly set the extension to be added to the filename.
     * \param[in] extension Extension to be added to the filename, including a "."
     *            (e.g., ".mhd") (std::string variant).
     */
    virtual void SetExtension(const std::string &extension);

    /**
     * \brief Get the extension to be added to the filename.
     * \return The file extension (e.g., ".mhd").
     */
    itkGetStringMacro(Extension);

    /**
     * \brief Reset the extension to the default value (".mhd").
     */
    void SetDefaultExtension();

    /**
     * \brief Set the file prefix for multi-file writing.
     * \warning Multiple file write is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePrefix);

    /**
     * \brief Get the file prefix.
     * \warning Multiple file write is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePrefix);

    /**
     * \brief Set the file pattern for multi-file writing.
     * \warning Multiple file write is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePattern);

    /**
     * \brief Get the file pattern.
     * \warning Multiple file write is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Set the 0th input object for the writer.
     * \param[in] input The mitk::Image to write.
     */
    void SetInput(mitk::Image *input);

    /**
     * \brief Return the list of possible file extensions for image data.
     * \return A vector of supported file extension strings.
     */
    std::vector<std::string> GetPossibleFileExtensions() override;

    /**
     * \brief Return the class name of the supported base data type.
     * \return The static class name of mitk::Image.
     */
    std::string GetSupportedBaseData() const override;

    /**
     * \brief Return the currently configured file extension.
     * \return The file extension string.
     */
    std::string GetFileExtension() override;

    /**
     * \brief Check whether the writer can write data from the given DataNode.
     * \param[in] node The DataNode to check. The node's data must be a mitk::Image.
     * \return \c true if the node contains a mitk::Image; \c false otherwise.
     */
    bool CanWriteDataType(DataNode * node) override;

    /**
     * \brief Return the MIME type of the file to be written.
     * \return The MIME type string (e.g., "image/vnd.mitk.image").
     */
    std::string GetWritenMIMEType() override;

    using Superclass::SetInput;

    /**
     * \brief Set a DataNode as input. Extracts the mitk::Image from the node.
     * \param[in] node The DataNode containing the image to write.
     */
    virtual void SetInput(DataNode * node);

    /**
     * \brief Get the 0th input image.
     * \return Pointer to the input mitk::Image, or \c nullptr if not set.
     */
    const mitk::Image *GetInput();

    /**
     * \brief Return the default filename for file save dialogs.
     * \return A default filename string (e.g., "GitHubIssue").
     */
    const char *GetDefaultFilename() override;

    /**
     * \brief Return the file dialog filter pattern.
     * \return A filter pattern string for file dialogs (e.g., "MITK Image (*.nrrd)").
     */
    const char *GetFileDialogPattern() override;

    /**
     * \brief Return the default file extension.
     * \return The default extension string (e.g., ".nrrd").
     */
    const char *GetDefaultExtension() override;

    /**
     * \brief Check whether this writer can write the given BaseData type.
     * \param[in] data The data object to check.
     * \return \c true if the data is a mitk::Image; \c false otherwise.
     */
    bool CanWriteBaseDataType(BaseData::Pointer data) override;

    /**
     * \brief Write the given BaseData object to file.
     * \param[in] data The data to write (must be a mitk::Image).
     */
    void DoWrite(BaseData::Pointer data) override;

    /**
     * \brief Enable or disable compression for the output file.
     * \param[in] useCompression If \c true, the output is compressed.
     */
    void SetUseCompression(bool useCompression);

  protected:
    /**
     * Constructor.
     */
    ImageWriter();

    /**
     * Virtual destructor.
     */
    ~ImageWriter() override;

    void GenerateData() override;

    virtual void WriteByITK(mitk::Image *image, const std::string &fileName);

    std::string m_FileName;

    std::string m_FileNameWithoutExtension;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    bool m_UseCompression;
  };
}

#endif
