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


#ifndef _MITK_IMAGE_WRITER__H_
#define _MITK_IMAGE_WRITER__H_

#include <MitkLegacyIOExports.h>
#include <mitkFileWriterWithInformation.h>


namespace mitk
{
class Image;
/**
 * @brief Writer for mitk::Image
 *
 * Uses the given extension (SetExtension) to decide the format to write
 * (.mhd is default, .pic, .tif, .png, .jpg supported yet).
 * @ingroup IO
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileWriterRegistry instead.
 */
class MitkLegacyIO_EXPORT ImageWriter :  public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( ImageWriter, mitk::FileWriter );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkWriterMacro;

    /**
     * Sets the filename of the file to write.
     * @param _arg the name of the file to write.
     */
    virtual void SetFileName (const char* fileName);
    virtual void SetFileName (const std::string& fileName);

    /**
     * @returns the name of the file to be written to disk.
     */
    itkGetStringMacro( FileName );

    /**
     * \brief Explicitly set the extension to be added to the filename.
     * @param _arg to be added to the filename, including a "."
     * (e.g., ".mhd").
     */
    virtual void SetExtension (const char* extension);
    virtual void SetExtension (const std::string& extension);

    /**
     * \brief Get the extension to be added to the filename.
     * @returns the extension to be added to the filename (e.g.,
     * ".mhd").
     */
    itkGetStringMacro( Extension );

    /**
     * \brief Set the extension to be added to the filename to the default
     */
    void SetDefaultExtension();

    /**
     * @warning multiple write not (yet) supported
     */
    itkSetStringMacro( FilePrefix );

    /**
     * @warning multiple write not (yet) supported
     */
    itkGetStringMacro( FilePrefix );

    /**
     * @warning multiple write not (yet) supported
     */
    itkSetStringMacro( FilePattern );

    /**
     * @warning multiple write not (yet) supported
     */
    itkGetStringMacro( FilePattern );

    /**
     * Sets the 0'th input object for the filter.
     * @param input the first input for the filter.
     */
    void SetInput( mitk::Image* input );

    //##Documentation
    //## @brief Return the possible file extensions for the data type associated with the writer
    virtual std::vector<std::string> GetPossibleFileExtensions();

    virtual std::string GetSupportedBaseData() const;

    /**
    * @brief Return the extension to be added to the filename.
    */
    virtual std::string GetFileExtension();

    /**
    * @brief Check if the Writer can write the Content of the
    */
    virtual bool CanWriteDataType( DataNode* );

    /**
    * @brief Return the MimeType of the saved File.
    */
    virtual std::string GetWritenMIMEType();

    using Superclass::SetInput;
    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataNode* );

    /**
     * @returns the 0'th input object of the filter.
     */
    const mitk::Image* GetInput();

    // FileWriterWithInformation methods
    virtual const char* GetDefaultFilename();
    virtual const char *GetFileDialogPattern();
    virtual const char *GetDefaultExtension();
    virtual bool CanWriteBaseDataType(BaseData::Pointer data);
    virtual void DoWrite(BaseData::Pointer data);

    void SetUseCompression( bool useCompression );

protected:

    /**
     * Constructor.
     */
    ImageWriter();

    /**
     * Virtual destructor.
     */
    virtual ~ImageWriter();

    virtual void GenerateData();

    virtual void WriteByITK(mitk::Image* image, const std::string& fileName);

    std::string m_FileName;

    std::string m_FileNameWithoutExtension;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    bool m_UseCompression;
};

}

#endif //_MITK_IMAGE_WRITER__H_
