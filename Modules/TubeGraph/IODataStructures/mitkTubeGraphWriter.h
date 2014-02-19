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

#ifndef __mitkTubeGraphWriter_h
#define __mitkTubeGraphWriter_h

#include "mitkFileWriterWithInformation.h"

#include "mitkTubeGraph.h"

#include <itkProcessObject.h>
#include <vtkPolyDataWriter.h>


namespace mitk
{

  /**
  * Writes tube graph to a file
  */
  class TubeGraphWriter : public FileWriterWithInformation
  {
  public:

    mitkClassMacro( TubeGraphWriter, FileWriterWithInformation );
    //mitkWriterMacro;
    virtual void Write()
    {
      if ( this->GetInput() == NULL )
      {
        itkExceptionMacro(<<"Write: Please specify an input!");
        return;
      }
      /* Fill in image information.*/
      this->UpdateOutputInformation();
      (*(this->GetInputs().begin()))->SetRequestedRegionToLargestPossibleRegion();
      this->PropagateRequestedRegion(NULL);
      this->UpdateOutputData(NULL);
    }

    virtual void Update()
    {
      Write();
    }

    itkNewMacro( Self );

    typedef TubeGraph InputType;

    /**
    * Sets the filename of the file to write.
    * @param FileName the name of the file to write.
    */
    itkSetStringMacro(FileName);

    /**
    * @returns the name of the file to be written to disk.
    */
    itkGetStringMacro(FileName);

    /**
    * @warning multiple write not (yet) supported
    */
    itkSetStringMacro(FilePrefix);

    /**
    * @warning multiple write not (yet) supported
    */
    itkGetStringMacro(FilePrefix);

    /**
    * @warning multiple write not (yet) supported
    */
    itkSetStringMacro(FilePattern);

    /**
    * @warning multiple write not (yet) supported
    */
    itkGetStringMacro(FilePattern);

    /**
    * Sets the input object for the filter.
    * @param input the tube graph to write to file.
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
    virtual const char * GetDefaultFilename() { return "TubeGraph.tsf"; }
    virtual const char * GetFileDialogPattern() { return "TubeGraph (*.tsf)"; }
    virtual const char * GetDefaultExtension() { return ".tsf"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) {
      return (dynamic_cast<InputType*>(data.GetPointer()) != NULL); }

    virtual void DoWrite(BaseData::Pointer data) {
      if (CanWriteBaseDataType(data)) {
        this->SetInput(dynamic_cast<InputType*>(data.GetPointer()));
        this->Update();
      }
    }

  protected:

    TubeGraphWriter();

    virtual ~TubeGraphWriter();

    virtual void GenerateData();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;

  };
} // end of namespace mitk
#endif //__mitkTubeGraphWriter_h
