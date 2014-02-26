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

#ifndef _MITK_CONTOURMODELSET_WRITER__H_
#define _MITK_CONTOURMODELSET_WRITER__H_

#include "ContourModelExports.h"
#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>

namespace mitk
{

  /**
  * @brief XML-based writer for mitk::ContourModelSet
  *
  * Uses the regular ContourModel writer to write each contour of the ContourModelSet to a single file.
  *
  * @ingroup PSIO
  * @ingroup Process
  */
  class ContourModel_EXPORT ContourModelSetWriter : public mitk::FileWriterWithInformation
  {
  public:

    mitkClassMacro( ContourModelSetWriter, mitk::FileWriter );

    mitkWriterMacro;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef mitk::ContourModelSet InputType;

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
    ContourModelSet* GetInput();

    /**
    * @param num the index of the desired output object.
    * @returns the n'th input object of the filter.
    */
    ContourModelSet* GetInput( const unsigned int& num );


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

    using Superclass::SetInput;

    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataNode* );

    /**
    * @returns whether the last write attempt was successful or not.
    */
    bool GetSuccess() const;

    /*++++++ FileWriterWithInformation methods +++++++*/
    virtual const char *GetDefaultFilename() { return "ContourModelSet.cnt_set"; }
    virtual const char *GetFileDialogPattern() { return "MITK ContourModelSet (*.cnt_set)"; }
    virtual const char *GetDefaultExtension() { return ".cnt_set"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) { return (dynamic_cast<mitk::ContourModelSet *>(data.GetPointer()) != NULL); };
    virtual void DoWrite(BaseData::Pointer data)
    {
      if (this->CanWriteBaseDataType(data))
      {
        this->SetInput(dynamic_cast<mitk::ContourModelSet *>(data.GetPointer()));
        this->Update();
      }
    }

  protected:

    /**
    * Constructor.
    */
    ContourModelSetWriter();

    /**
    * Virtual destructor.
    */
    virtual ~ContourModelSetWriter();


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


    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    bool m_Success;

  };

}


#endif
