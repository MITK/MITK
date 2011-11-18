/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-08-27 17:18:46 +0200 (Mi, 27 Aug 2008) $
Version:   $Revision: 15096 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkFiberBundleXWriter_h
#define __mitkFiberBundleXWriter_h

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include "mitkFiberBundleX.h"
#include <vtkPolyDataWriter.h>


namespace mitk
{

/**
 * Writes fiber bundles to a file
 * @ingroup Process
 */
class FiberBundleXWriter : public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( FiberBundleXWriter, mitk::FileWriterWithInformation );

    //mitkWriterMacro;

    virtual void Write()
    {
      if ( this->GetInput() == NULL )
    {
      itkExceptionMacro(<<"Write:Please specify an input!");
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

    typedef mitk::FiberBundleX InputType;

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
    void SetInputFiberBundleX( InputType* input );

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
    virtual const char * GetDefaultFilename() { return "FiberBundle.fib"; }
    virtual const char * GetFileDialogPattern() { return "Fiber Bundle (*.fib *.vtk)"; }
    virtual const char * GetDefaultExtension() { return ".fib"; }
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) { return (dynamic_cast<mitk::FiberBundleX*>(data.GetPointer()) != NULL); };
    virtual void DoWrite(BaseData::Pointer data) {
      if (CanWriteBaseDataType(data)) {
        this->SetInputFiberBundleX(dynamic_cast<mitk::FiberBundleX*>(data.GetPointer()));
        this->Update();
      }
    };

    static const char* XML_GEOMETRY;

    static const char* XML_MATRIX_XX;

    static const char* XML_MATRIX_XY;

    static const char* XML_MATRIX_XZ;

    static const char* XML_MATRIX_YX;

    static const char* XML_MATRIX_YY;

    static const char* XML_MATRIX_YZ;

    static const char* XML_MATRIX_ZX;

    static const char* XML_MATRIX_ZY;

    static const char* XML_MATRIX_ZZ;

    static const char* XML_ORIGIN_X;

    static const char* XML_ORIGIN_Y;

    static const char* XML_ORIGIN_Z;

    static const char* XML_SPACING_X;

    static const char* XML_SPACING_Y;

    static const char* XML_SPACING_Z;

    static const char* XML_SIZE_X;

    static const char* XML_SIZE_Y;

    static const char* XML_SIZE_Z;

    static const char* XML_FIBER_BUNDLE;

    static const char* XML_FIBER;

    static const char* XML_PARTICLE;

    static const char* XML_ID;

    static const char* XML_POS_X;

    static const char* XML_POS_Y;

    static const char* XML_POS_Z;

    static const char* VERSION_STRING;

    static const char* XML_FIBER_BUNDLE_FILE;

    static const char* XML_FILE_VERSION;

    static const char* XML_NUM_FIBERS;

    static const char* XML_NUM_PARTICLES;

    static const char* ASCII_FILE;

    static const char* FILE_NAME;

protected:

    FiberBundleXWriter();

    virtual ~FiberBundleXWriter();

    virtual void GenerateData();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;

};


} // end of namespace mitk

#endif //__mitkFiberBundleXWriter_h
