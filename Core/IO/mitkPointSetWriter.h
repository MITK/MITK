#ifndef _MITK_POINT_SET_WRITER__H_
#define _MITK_POINT_SET_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriter.h>
#include <mitkPointSet.h>

namespace mitk
{

class PointSetWriter : public itk::ProcessObject, public mitk::FileWriter
{
public:

    mitkClassMacro( PointSetWriter, itk::ProcessObject );

    itkNewMacro( Self );

    typedef mitk::PointSet InputType;

    typedef InputType::Pointer InputTypePointer;

    /**
     * Sets the filename of the file to write.
     * @param FileName the name of the file to write.
     */
    itkSetStringMacro( FileName );

    /**
     * @returns the name of the file to be written to disk.
     */
    itkGetStringMacro( FileName );

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

    void SetInput( InputType* );

    void SetInput( const unsigned int& num, InputType* );

    PointSet* GetInput();

    PointSet* GetInput( const unsigned int& num );

    void Write();

protected:

    /**
     * Constructor.
     */
    PointSetWriter();

    /**
     * Virtual destructor.
     */
    virtual ~PointSetWriter();


    /**
     * Determines of which file type a given file is and calls the 
     * appropriate reader function.
     */
    virtual void GenerateData();


    /**
     * Resizes the number of inputs of the writer.
     * The outputs are initialized by empty PointSets
     * @param num the new number of inputs
     */
    virtual void ResizeInputs( const unsigned int& num );

    template < typename T>
    std::string ConvertToString( T value );

    void WriteXML( mitk::PointSet* pointSet, std::ofstream& out );

    void WriteXMLHeader( std::ofstream &file );
    
    /** Write a start element tag */
    void WriteStartElement( const char *const tag, std::ofstream &file );

    /**
     * Write an end element tag
     * End-Elements following character data should pass indent = false.
     */
    void WriteEndElement( const char *const tag, std::ofstream &file, const bool& indent = true );

    /** Write character data inside a tag. */
    void WriteCharacterData( const char *const data, std::ofstream &file );

    /** Write a start element tag */
    void WriteStartElement( std::string &tag, std::ofstream &file );

    /** Write an end element tag */
    void WriteEndElement( std::string &tag, std::ofstream &file, const bool& indent = true );

    /** Write character data inside a tag. */
    void WriteCharacterData( std::string &data, std::ofstream &file );

    void WriteIndent( std::ofstream& file );

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    unsigned int m_IndentDepth;

    unsigned int m_Indent;
    
public:

    static const char* XML_POINT_SET;
    
    static const char* XML_POINT_SET_FILE;
    
    static const char* XML_FILE_VERSION;
    
    static const char* XML_POINT;
    
    static const char* XML_ID;
    
    static const char* XML_X;
    
    static const char* XML_Y;
    
    static const char* XML_Z;
    
    static const char* VERSION_STRING;

};



}


#endif
