/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12. Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PlanarFigure_WRITER__H_
#define _MITK_PlanarFigure_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriter.h>
#include <mitkPlanarFigure.h>

namespace mitk
{

  /**
  * @brief XML-based writer for mitk::PlanarFigures 
  *
  * XML-based writer for mitk::PlanarFigures.
  * @ingroup Process
  */
  class MITKEXT_CORE_EXPORT PlanarFigureWriter : public mitk::FileWriter
  {
  public:

    mitkClassMacro( PlanarFigureWriter, mitk::FileWriter );

    mitkWriterMacro;

    itkNewMacro( Self );

    typedef mitk::PlanarFigure InputType;

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
    PlanarFigure* GetInput();

    /**
    * @param num the index of the desired output object.
    * @returns the n'th input object of the filter. 
    */
    PlanarFigure* GetInput( const unsigned int& num );


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
    virtual bool CanWriteDataType( DataTreeNode* );

    /**
    * @brief Return the MimeType of the saved File.
    */
    virtual std::string GetWritenMIMEType();

    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataTreeNode* );

    /**
    * @returns whether the last write attempt was successful or not.
    */
    itkGetConstMacro(Success, bool);

  protected:

    /**
    * Constructor.
    */
    PlanarFigureWriter();

    /**
    * Virtual destructor.
    */
    virtual ~PlanarFigureWriter();


    /**
    * Writes the a .pf file in xml format that contains all input planar figures
    */
    virtual void GenerateData();


    /**
    * Resizes the number of inputs of the writer.
    * The inputs are initialized by empty PlanarFigures
    * @param num the new number of inputs
    */
    virtual void ResizeInputs( const unsigned int& num );

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;
    std::string m_Extension;
    std::string m_MimeType;
    bool m_Success;
  };
}
#endif
