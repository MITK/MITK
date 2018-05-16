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

#ifndef _MITK_NRRDTBSSROIVOL_WRITER__H_
#define _MITK_NRRDTBSSROIVOL_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkTbssRoiImage.h>
#include "MitkQuantificationExports.h"

namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */

class MITKQUANTIFICATION_EXPORT NrrdTbssRoiImageWriter : public mitk::FileWriterWithInformation
{
public:

    typedef itk::Image<unsigned char,3>     ImageType;

    mitkClassMacro( NrrdTbssRoiImageWriter, mitk::FileWriterWithInformation )

    mitkWriterMacro

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef mitk::TbssRoiImage InputType;

    /**
     * Sets the filename of the file to write.
     * @param FileName the nameInputType of the file to write.
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

    /**image
     * Sets the input object for the filter.
     * @param input the diffusion volumes to write to file.
     */
    using ProcessObject::SetInput;
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
    std::vector<std::string> GetPossibleFileExtensions() override;

    std::string GetSupportedBaseData() const override;

    // FileWriterWithInformation methods
    const char * GetDefaultFilename() override { return "TbssRoiImage.roi"; }
    const char * GetFileDialogPattern() override { return "Tbss Roi Images (*.roi)"; }
    const char * GetDefaultExtension() override { return ".roi"; }
    bool CanWriteBaseDataType(BaseData::Pointer data) override
    {
      return (dynamic_cast<mitk::TbssRoiImage*>(data.GetPointer()) != nullptr);
    }


    void DoWrite(BaseData::Pointer data) override {
      if (CanWriteBaseDataType(data)) {
        this->SetInput(dynamic_cast<mitk::TbssRoiImage*>(data.GetPointer()));
        this->Update();
      }
    }

protected:

    NrrdTbssRoiImageWriter();

    ~NrrdTbssRoiImageWriter() override;

    void GenerateData() override;

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;

};


} // end of namespace mitk

#endif
