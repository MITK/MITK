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


#ifndef _MITK_PlanarFigure_WRITER__H_
#define _MITK_PlanarFigure_WRITER__H_

#include <itkProcessObject.h>
#include <MitkPlanarFigureExports.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkPlanarFigure.h>

class TiXmlElement;
namespace mitk
{

  /**
  * @brief XML-based writer for mitk::PlanarFigures
  *
  * XML-based writer for mitk::PlanarFigures.
  * @ingroup MitkPlanarFigureModule
  */
  class MITKPLANARFIGURE_EXPORT PlanarFigureWriter : public mitk::FileWriterWithInformation
  {
  public:

    mitkClassMacro( PlanarFigureWriter, mitk::FileWriter );

    mitkWriterMacro;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

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

    using Superclass::SetInput;

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
    virtual bool CanWriteDataType( DataNode* );

    /**
    * @brief Return the MimeType of the saved File.
    */
    virtual std::string GetWritenMIMEType();

    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput( DataNode* );

    virtual std::string GetSupportedBaseData() const;

    /**
    * @returns whether the last write attempt was successful or not.
    */
    itkGetConstMacro(Success, bool);


    virtual const char * GetDefaultFilename() { return "PlanarFigure.pf"; }
    virtual const char * GetFileDialogPattern() { return "Planar Figure Files (*.pf)"; }
    virtual const char * GetDefaultExtension() { return ".pf"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data)
    {
      return dynamic_cast<mitk::PlanarFigure*>( data.GetPointer() );
    }
    virtual void DoWrite(BaseData::Pointer data) {
      if (CanWriteBaseDataType(data)) {
        this->SetInput(dynamic_cast<mitk::PlanarFigure*>(data.GetPointer()));
        this->Update();
      }
    }


    /**
    @brief CAUTION: It's up to the user to call this function to release the
    memory buffer after use in case the file writer has written to its memory array.
    See mitkFileWriter base class. */
    virtual void         ReleaseMemory();

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

    /**Documentation
    * \brief creates a TinyXML element that contains x, y, and z values
    *
    * \param[in] name the name of the XML element
    * \param[in] v the vector or point that contains the x, y and z values
    * \return returns a TiXmlElement named name and three attributes x, y and z.
    */
    TiXmlElement* CreateXMLVectorElement(const char* name, itk::FixedArray<mitk::ScalarType, 3> v);

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;
    std::string m_Extension;
    std::string m_MimeType;
    bool m_Success;
  };
}

#endif
