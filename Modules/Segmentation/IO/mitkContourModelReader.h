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
#ifndef _MITK_CONTOURMODEL_READER__H_
#define _MITK_CONTOURMODEL_READER__H_

#include "SegmentationExports.h"
#include <mitkContourModel.h>
#include <mitkContourModelSource.h>
#include <mitkFileReader.h>
#include <string>
#include <stack>
#include <vtkXMLParser.h>
#include <tinyxml.h>

namespace mitk
{
/**
 * @brief
 * @ingroup PSIO
 * @ingroup IO
*/
class Segmentation_EXPORT ContourModelReader : public ContourModelSource, public FileReader
{
public:

    mitkClassMacro( ContourModelReader, FileReader );

    itkNewMacro( Self );

    /**
     * @brief Sets the filename of the file to be read
     * @param _arg the filename of the point set xml-file
     */
    itkSetStringMacro( FileName );

    /**
     * @brief Returns the filename of the point set xml-file.
     * @returns the filename of the point set xml-file.
     */
    itkGetStringMacro( FileName );

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePattern );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePattern );

    static bool CanReadFile(const std::string filename, const std::string filePrefix,
      const std::string filePattern);

    /**
     * @returns whether the last read attempt was successful or not.
     */
    bool GetSuccess() const;

protected:

    /**
     * Constructor
     */
    ContourModelReader();

    /**
     * Virtual destructor
     */
    virtual ~ContourModelReader();

    /**
     * Actually reads the point sets from the given file
     */
    virtual void GenerateData();

    virtual void ReadPoints(mitk::ContourModel::Pointer newContourModel,
      TiXmlElement* currentTimeSeries, unsigned int currentTimeStep);

    /**
     * Does nothing in the current implementation
     */
    virtual void GenerateOutputInformation();

    /**
     * Resizes the output-objects according to the given number.
     * @param num the new number of output objects.
     */
    virtual void ResizeOutputs( const unsigned int& num );

    /**
     * Checks if the given file has appropriate
     * read access.
     * @returns true if the file exists and may be read
     *          or false otherwise.
     */
    virtual int CanReadFile (const char *name);


    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;

};

}


#endif
