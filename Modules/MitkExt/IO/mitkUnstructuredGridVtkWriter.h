/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 10863 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_
#define _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_


#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>

#include "mitkUnstructuredGrid.h"

namespace mitk
{

/**
 * @brief VTK-based writer for mitk::UnstructuredGrid
 *
 * The mitk::UnstructuredGrid is written using the VTK-writer-type provided as the
 * template argument. If the mitk::UnstructuredGrid contains multiple points of
 * time, multiple files are written. The life-span (time-bounds) of each
 * each point of time is included in the filename according to the 
 * following scheme: 
 * &lt;filename&gt;_S&lt;timebounds[0]&gt;E&lt;timebounds[1]&gt;_T&lt;framenumber&gt;
 * (S=start, E=end, T=time).
 * Writing of multiple files according to a given filename pattern is not
 * yet supported.
 * @ingroup Process
*/
template<class VTKWRITER>
class UnstructuredGridVtkWriter : public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( UnstructuredGridVtkWriter, mitk::FileWriterWithInformation );

    itkNewMacro( Self );

    mitkWriterMacro;

    /**
     * Sets the filename of the file to write.
     * @param _arg the name of the file to write.
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
    void SetInput( mitk::UnstructuredGrid* input );

    /**
     * @returns the 0'th input object of the filter.
     */
    const UnstructuredGrid* GetInput();

    /**
     * Returns false if an error happened during writing
     */
    itkGetMacro( Success, bool );

    /**
    * @brief Return the possible file extensions for the data type associated with the writer
    */
    virtual std::vector<std::string> GetPossibleFileExtensions();

    // FileWriterWithInformation methods
    virtual const char * GetDefaultFilename();
    virtual const char * GetFileDialogPattern();
    virtual const char * GetDefaultExtension();
    virtual bool CanWriteBaseDataType(BaseData::Pointer data);
    virtual void DoWrite(BaseData::Pointer data);

protected:

    /**
     * Constructor.
     */
    UnstructuredGridVtkWriter();

    /**
     * Virtual destructor.
     */
    virtual ~UnstructuredGridVtkWriter();

    void ExecuteWrite(VTKWRITER* vtkWriter);

    virtual void GenerateData();
      
    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;
};

}

#include "mitkUnstructuredGridVtkWriter.txx"

#endif // _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_
