#ifndef _MITK_POINT_SET_READER__H_
#define _MITK_POINT_SER_READER__H_

#include <mitkPointSetSource.h>
#include <mitkFileReader.h>
#include <string>
#include <stack>
#include <vtkXMLParser.h>

namespace mitk
{

class PointSetReader: public mitk::PointSetSource, public mitk::FileReader
{
public:
    mitkClassMacro( PointSetReader, PointSetSource );

    itkNewMacro( Self );

    /**
     * @brief Sets the filename of the file to be read
     * @param FileName the filename of the vessel graph file
     */
    itkSetStringMacro( FileName );

    /**
     * @brief Returns the filename of the vessel graph file.
     * @returns the filename of the vessel graph file.
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
    
    void Read();
    
protected:


    PointSetReader();

    virtual ~PointSetReader();
    
    virtual void GenerateData();
    
    virtual void GenerateOutputInformation();
    
    virtual void ResizeOutputs( const unsigned int& num );
    
    virtual int CanReadFile (const char *name);
    
    
    std::string m_FileName;
    
    std::string m_FilePrefix;
    
    std::string m_FilePattern;  
    
};
    
}


#endif
