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

#ifndef _MITK_NRRDDIFFVOL_WRITER__H_
#define _MITK_NRRDDIFFVOL_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkLabelSetImage.h>

namespace mitk
{

/**
 * Writes a labelset to a file
 * @ingroup Process
 */
template < class TPixelType >
class NrrdLabelSetImageWriter : public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( NrrdLabelSetImageWriter, mitk::FileWriterWithInformation );

    mitkWriterMacro;

    itkNewMacro( Self );

//    typedef mitk::LabelSetImage<TPixelType> InputType;
    typedef mitk::LabelSetImage InputType;

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
     * Sets the input object for the filter.
     * @param input the diffusion volumes to write to file.
     */
    void SetInput( InputType* input );

    /**
     * @returns the 0'th input object of the filter.
     */
    InputType* GetInput();

    /**
     * Returns false if an error happened during writing
     */
    itkGetMacro( Success, bool );

    /**
    * @return possible file extensions for the data type associated with the writer
    */
    virtual std::vector<std::string> GetPossibleFileExtensions();

    // FileWriterWithInformation methods
    virtual const char * GetDefaultFilename() { return "LabelsetImages.lset"; }
    virtual const char * GetFileDialogPattern() { return "Label Set Images (*.lset)"; }
    virtual const char * GetDefaultExtension() { return ".lset"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data)
    {
      //return (dynamic_cast<mitk::LabelSetImage<TPixelType>*>(data.GetPointer()) != NULL);
        return (dynamic_cast<mitk::LabelSetImage*>(data.GetPointer()) != NULL);
    }


    virtual void DoWrite(BaseData::Pointer data) {
      if (CanWriteBaseDataType(data)) {
        //this->SetInput(dynamic_cast<mitk::LabelSetImage<TPixelType>*>(data.GetPointer()));
        this->SetInput(dynamic_cast<mitk::LabelSetImage*>(data.GetPointer()));
        this->Update();
      }
    }

protected:

    NrrdLabelSetImageWriter();

    virtual ~NrrdLabelSetImageWriter();

    virtual void GenerateData();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;

};


} // end of namespace mitk

#include "mitkNrrdLabelSetImageWriter.cpp"

#endif
