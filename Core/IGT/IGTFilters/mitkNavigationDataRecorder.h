/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_NavigationDataRecorder_H
#define _MITK_NavigationDataRecorder_H

#include <itkProcessObject.h>
#include "mitkNavigationData.h"

#include <iostream>

namespace mitk
{

 /**Documentation
 * \brief This class records NavigationData objects.
 *
 * The output of this class is formated as a XML document. 
 *
 * Internal this class uses streams for recording NavigationData objects. Therefore different types of output are possible 
 * and can be set with the SetOutputMode() method. The default output is directed to the console. If you want to save into a  
 * file you have to set a file name and the path. The recording is started with the call of the method StartRecording(). Now 
 * every Update() stores the current state of the added NavigationDatas. With StopRecording() the stream is stopped. With
 * another call of StartRecording() the output is written to a new file with incremented filename counter. 
 *
 * \warning At the moment there is no check if the file is already existing and this class will override existing files.
 * \ingroup IGT
 */

class NavigationDataRecorder : public itk::ProcessObject
{
public:
    mitkClassMacro( NavigationDataRecorder, itk::ProcessObject );

    itkNewMacro( Self );

    /**
    * \brief sets the file name for the OutputMode NormalFile and ZipFile
    *
    * Any extensions will be cut
    * \warning existing files will be overridden
    * \warning do not use "." in file names at the end
    */
    itkSetStringMacro(FileName);
    
    /**
    * \brief Returns the file name of the recording file (in OutputMode NormalFile and ZipFile)
    */
    itkGetStringMacro(FileName);

    /**
    * \brief Adds the input NavigationDatas
    */
    virtual void AddNavigationData(const NavigationData* nd);

    /**Documentation
    * \brief Starts the recording with the presetted OutputMode 
    * this method calls StartRecording(std::ostream*)
    */
    void StartRecording();

    /**Documentation
    * \brief Starts the recording with an own preinitialized stream
    */
    void StartRecording(std::ostream* stream);

    /**Documentation
    * \brief Stops the recording and closes the stream
    */
    void StopRecording();

    /**Documentation
    * \brief Every call of update causes one line for each added NavigationData in the output if the recording was started
    */
    virtual void Update();

    /**Documentation
    * \brief Determines where the output is directed to
    * 
    * Console:    std::cout
    * NormalFile: std::ofstream
    * ZipFile:    Not supported yet -> std::cout
    */
    enum RecordingMode
    {
      Console,
      NormalFile,
      ZipFile
    };

    /**Documentation
    * \brief Sets the recording mode which causes different types of output streams
    * see enum RecordingMode
    */
    void SetRecordingMode(RecordingMode mode);

protected:

    /**Documentation
    * \brief filter execute method here it is not used
    *
    */
    virtual void GenerateData();

    NavigationDataRecorder();
    
    virtual ~NavigationDataRecorder();

    std::string m_FileName; ///< stores the file name and path

    unsigned int m_NumberOfInputs; ///< counts the numbers of added input NavigationDatas

    std::ostream* m_Stream; ///< the output stream

    RecordingMode m_RecordingMode; ///< stores the mode see enum RecordingMode

    bool m_Recording; ///< indicates whether the recording is started or not

    unsigned int m_NumberOfRecordedFiles; ///< necessary for the naming of the file if there is more than one start-stop cycle

};

}
#endif // #define _MITK_POINT_SET_SOURCE_H

