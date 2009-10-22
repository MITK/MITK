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

#ifndef _MITK_NRRDQBI_WRITER__H_
#define _MITK_NRRDQBI_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkQBallImage.h>


namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
class NrrdQBallImageWriter : public mitk::FileWriterWithInformation
{
public:

    mitkClassMacro( NrrdQBallImageWriter, mitk::FileWriterWithInformation );

    mitkWriterMacro;

    itkNewMacro( Self );
    
    typedef mitk::QBallImage InputType;
    
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
    virtual const char * GetDefaultFilename() { return "QBalls.qbi"; }
    virtual const char * GetFileDialogPattern() { return "Q-Ball Images (*.qbi *.hqbi"; }
    virtual const char * GetDefaultExtension() { return ".qbi"; }
    virtual bool CanWriteDataType(BaseData::Pointer data) { return (dynamic_cast<mitk::QBallImage*>(data.GetPointer()) != NULL); };  
    virtual void DoWrite(BaseData::Pointer data) { 
      if (CanWriteDataType(data)) {
        this->SetInput(dynamic_cast<mitk::QBallImage*>(data.GetPointer())); 
        this->Update(); 
      }
    };

protected:
        
    NrrdQBallImageWriter();

    virtual ~NrrdQBallImageWriter();

    virtual void GenerateData();
    
    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
    
    bool m_Success;
            
};    
       

} // end of namespace mitk

#endif //_MITK_NRRDQBI_WRITER__H_
