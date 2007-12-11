/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef _MITK_POINT_SET_READER__H_
#define _MITK_POINT_SET_READER__H_

#include <mitkPointSetSource.h>
#include <mitkFileReader.h>
#include <string>
#include <stack>
#include <vtkXMLParser.h>

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
 * @note loading point sets from multiple files according to a given file pattern
 * is not yet supported!
 * @ingroup IO
*/
class PointSetReader: public PointSetSource, public FileReader
{
public:

    mitkClassMacro( PointSetReader, FileReader );

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
    virtual ~PointSetReader();
    
    /**
     * Actually reads the point sets from the given file
     */
    virtual void GenerateData();
    
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
