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

#ifndef _MITK_CONTOURMODEL_WRITER__H_
#define _MITK_CONTOURMODEL_WRITER__H_

#include "SegmentationExports.h"
#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkContourModel.h>

namespace mitk
{

/**
 * @brief XML-based writer for mitk::ContourModels
 *
 * XML-based writer for mitk::ContourModels. Multiple ContourModels can be written in
 * a single XML file by simply setting multiple inputs to the filter.
 *
 * @ingroup PSIO
 * @ingroup Process
 */
class Segmentation_EXPORT ContourModelWriter : public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( ContourModelWriter, mitk::FileWriter );

    mitkWriterMacro;

    itkNewMacro( Self );

    typedef mitk::ContourModel InputType;

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

    /**
     * Sets the 0'th input object for the filter.
     * @param input the first input for the filter.
     */
    void SetInput( InputType* input );

    /**
     * Sets the n'th input object for the filter. If num is
     * larger than GetNumberOfInputs() the number of inputs is
     * resized appropriately.
     * @param input the n'th input for the filter.
     */
    void SetInput( const unsigned int& num, InputType* input);

    /**
     * @returns the 0'th input object of the filter.
     */
    ContourModel* GetInput();

    /**
     * @param num the index of the desired output object.
     * @returns the n'th input object of the filter.
     */
    ContourModel* GetInput( const unsigned int& num );


    /**
    * @brief Return the possible file extensions for the data type associated with the writer
    */
    virtual std::vector<std::string> GetPossibleFileExtensions();

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

    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataNode* );

    /**
    * @returns whether the last write attempt was successful or not.
    */
    bool GetSuccess() const;

    /*++++++ FileWriterWithInformation methods +++++++*/
    virtual const char *GetDefaultFilename() { return "ContourModel.cnt"; }
    virtual const char *GetFileDialogPattern() { return "MITK ContourModel (*.cnt)"; }
    virtual const char *GetDefaultExtension() { return ".cnt"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) { return (dynamic_cast<mitk::ContourModel *>(data.GetPointer()) != NULL); };
    virtual void DoWrite(BaseData::Pointer data)
    {
      if (this->CanWriteBaseDataType(data))
      {
        this->SetInput(dynamic_cast<mitk::ContourModel *>(data.GetPointer()));
        this->Update();
      }
    }

protected:

    /**
     * Constructor.
     */
    ContourModelWriter();

    /**
     * Virtual destructor.
     */
    virtual ~ContourModelWriter();


    /**
     * Writes the XML file
     */
    virtual void GenerateData();


    /**
     * Resizes the number of inputs of the writer.
     * The inputs are initialized by empty ContourModels
     * @param num the new number of inputs
     */
    virtual void ResizeInputs( const unsigned int& num );


    /**
     * Converts an arbitrary type to a string. The type has to
     * support the << operator. This works fine at least for integral
     * data types as float, int, long etc.
     * @param value the value to convert
     * @returns the string representation of value
     */
    template < typename T>
    std::string ConvertToString( T value );

    /**
     * Writes an XML representation of the given point set to
     * an outstream. The XML-Header an root node is not included!
     * @param contourModel the point set to be converted to xml
     * @param out the stream to write to.
     */
    void WriteXML( mitk::ContourModel* contourModel, std::ofstream& out );

    /**
    * Writes the geometry information of the TimeSlicedGeometry to an outstream.
    * The root tag is not included.
    * @param geometry the TimeSlicedGeometry of the contour.
    * @param the stream to write to.
    */
    void WriteGeometryInformation( mitk::TimeSlicedGeometry* geometry, std::ofstream& out );

    /**
     * Writes an standard xml header to the given stream.
     * @param file the stream in which the header is written.
     */
    void WriteXMLHeader( std::ofstream &file );

    /** Write a start element tag */
    void WriteStartElement( const char *const tag, std::ofstream &file );

    void WriteStartElementWithAttribut( const char *const tag, std::vector<std::string> attributes, std::vector<std::string> values, std::ofstream &file );

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

    /** Writes empty spaces to the stream according to m_IndentDepth and m_Indent */
    void WriteIndent( std::ofstream& file );

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    unsigned int m_IndentDepth;

    unsigned int m_Indent;

    bool m_Success;



public:

    static const char* XML_CONTOURMODEL;

    static const char* XML_HEAD;

    static const char* XML_GEOMETRY_INFO;

    static const char* XML_DATA;

    static const char* XML_TIME_STEP;

    static const char* XML_CONTROL_POINTS;

    static const char* XML_POINT;

    static const char* XML_X;

    static const char* XML_Y;

    static const char* XML_Z;


};



}


#endif
