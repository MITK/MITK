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


#ifndef _MITK_IMAGE_WRITER__H_
#define _MITK_IMAGE_WRITER__H_

#include <iomanip>

#include <itkProcessObject.h>
#include <mitkFileWriter.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>

namespace mitk
{
class Image;
/**
 * @brief Writer for mitk::Image
 *
 * Uses the given extension (SetExtension) to decide the format to write
 * (.mhd is default, .pic, .tif, .png, .jpg supported yet).
 * @ingroup IO
 */
class ImageWriter :  public mitk::FileWriter
{
public:

    mitkClassMacro( ImageWriter, mitk::FileWriter );

    itkNewMacro( Self );

    mitkWriterMacro;

    /**
     * Sets the filename of the file to write.
     * @param _arg the name of the file to write.
     */
    itkSetStringMacro( FileName );

    /**
     * @returns the name of the file to be written to disk.
     */
    itkGetStringMacro( FileName );

    /**
     * \brief Explicitly set the extension to be added to the filename.
     * @param _arg to be added to the filename, including a "." 
     * (e.g., ".mhd").
     */
    itkSetStringMacro( Extension );

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

    /**
    * @brief Return the extension to be added to the filename.
    */
    virtual std::string GetFileExtension();

    /**
    * @brief Check if the Writer can write the Content of the 
    */
    virtual bool CanWrite( DataTreeNode* );

    /**
    * @brief Return the MimeType of the saved File.
    */
    virtual std::string GetWritenMIMEType();

    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataTreeNode* );

    /**
     * @returns the 0'th input object of the filter.
     */
    const mitk::Image* GetInput();

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

    virtual void WriteByITK(mitk::Image* image, const std::string& filename);

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;
};

}

#endif //_MITK_IMAGE_WRITER__H_
