#ifndef _MITK_POINT_SET_READER__H_
#define _MITK_POINT_SER_READER__H_

#include <mitkPointSetSource.h>
#include <mitkFileReader.h>
#include <string>
#include <stack>
#include <vtkXMLParser.h>

namespace mitk
{
/**
 * @brief reads xml representations of mitk::PointSets from a file
 * @ingroup process
 * Reader for xml files containing one or multiple xml represenations of 
 * mitk::PointSets. If multiple mitk::PointSets are stored in one file,
 * these are assigned to multiple outputs of the filter. The number of point
 * sets which have be read can be retrieven by a call to GetNumberOfOutputs() 
 * after the pipeline update().
 * Note: loading point sets from multiple files according to a given file pattern
 * is not yet supported!
 */
class PointSetReader: public mitk::PointSetSource, public mitk::FileReader
{
public:
    mitkClassMacro( PointSetReader, PointSetSource );

    itkNewMacro( Self );

    /**
     * @brief Sets the filename of the file to be read
     * @param FileName the filename of the point set xml-file
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
    
    /**
     * Explicitly calls GenerateData(), since the pipelines Update()
     * is broken. This will be fixed.
     */
    void Read();
    
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
    
};
    
}


#endif
