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

#ifndef _MITK_NavigationDataRecorderDeprecated_H
#define _MITK_NavigationDataRecorderDeprecated_H

#include <itkProcessObject.h>
#include "mitkNavigationData.h"

#include <iostream>

#include <mitkRealTimeClock.h>

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

  class MitkIGT_EXPORT NavigationDataRecorderDeprecated : public itk::ProcessObject
  {
  public:
    mitkClassMacro( NavigationDataRecorderDeprecated, itk::ProcessObject );

    itkNewMacro( Self );

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
    * \brief Determines the output format
    *
    * xml:  XML format, also default, can be read by NavigationDataPlayer
    * csv:  use to export in excel, matlab, etc.
    */
    enum OutputFormatEnum
    {
      xml,
      csv
    };

    /**
    * \brief sets the file name for the OutputMode NormalFile and ZipFile
    *
    * Any extensions will be cut
    * \warning existing files will be overridden
    * \warning do not use "." in file names at the end
    */
    DEPRECATED( itkSetStringMacro(FileName));

    /**
    * \brief Returns the file name of the recording file (in OutputMode NormalFile and ZipFile)
    */
    DEPRECATED( itkGetStringMacro(FileName));

    /**
    * \brief If true the recorder will never overwrite a file
    */
    DEPRECATED( itkSetMacro(DoNotOverwriteFiles,bool));

    /**
    * \brief Returns whether the NavigationDataRecorderDeprecated is recording or not
    */
    DEPRECATED( itkGetMacro(Recording,bool));

    /**
    * \brief Returns the recording mode
    */
    DEPRECATED( itkGetMacro(RecordingMode,RecordingMode));

    /**
    * \brief Returns the number of data sets / frames which were recorded by the NavigationDataRecorderDeprecated since start
    */
    DEPRECATED( itkGetMacro(RecordCounter,int));

    /**
    * \brief Sets a limit of recorded data sets / frames. Recording will be stopped if the number is reached. -1 disables the limit, -1 is default value as well.
    */
    DEPRECATED( itkSetMacro(RecordCountLimit,int));

    /**
    * \brief Adds the input NavigationDatas
    */
    DEPRECATED( virtual void AddNavigationData(const NavigationData* nd));

    ///
    /// set an additional attribute for a specified navigation data
    /// this will be written for each navigation data and may be
    /// updated before calling Update()
    ///
    DEPRECATED( void SetAdditionalAttribute( const NavigationData* nd, const std::string& attributeName
      , const std::string& attributeValue ));
    DEPRECATED(void RemoveAdditionalAttribute( const NavigationData* nd ));

    /**
    * Documentation
    * \brief Starts the recording with the presetted OutputMode.
    *        This method calls StartRecording(std::ostream*).
    *        Does nothing if the recorder is already recording and
    *        the method StartRecording is called again.
    * @throw mitk::IGTException Throws an exception if no file name or file path is set.
    */
    DEPRECATED( void StartRecording());

    /**
    * Documentation
    * \brief Starts the recording with an own preinitialized stream
    * Does nothing if it is already recording and method StartRecorded is called
    * @throw mitk::IGTException Throws an exception if the stream is not good.
    */
    DEPRECATED( void StartRecording(std::ostream* stream));

    /**Documentation
    * \brief Stops the recording and closes the stream
    */
    DEPRECATED( void StopRecording());

    /**Documentation
    * \brief Every call of update causes one line for each added NavigationData in the output if the recording was started
    */

    DEPRECATED( virtual void Update());

    /**Documentation
    * \brief Sets the recording mode which causes different types of output streams
    * see enum RecordingMode
    */
    DEPRECATED( void SetRecordingMode(RecordingMode mode));

    /**Documentation
    * \brief Sets the output format which causes different formats of output streams. The XML format is default.
    *  Also see enum OutputFormat for more information.
    */
    DEPRECATED( itkSetMacro(OutputFormat,mitk::NavigationDataRecorderDeprecated::OutputFormatEnum));

  protected:

    /**Documentation
    * \brief filter execute method here it is not used
    *
    */
    virtual void GenerateData();

    NavigationDataRecorderDeprecated();

    virtual ~NavigationDataRecorderDeprecated();

    std::string m_FileName; ///< stores the file name and path

    unsigned int m_NumberOfInputs; ///< counts the numbers of added input NavigationDatas

    std::ostream* m_Stream; ///< the output stream

    bool m_StreamMustBeDeleted;

    RecordingMode m_RecordingMode; ///< stores the mode see enum RecordingMode

    OutputFormatEnum m_OutputFormat; ///< stores the output format; see enum OutputFormat

    bool m_Recording; ///< indicates whether the recording is started or not

    int m_RecordCounter; ///< counts the number of frames which are recorded since StartRecording

    int m_RecordCountLimit; ///< limits the number of frames, recording will be stopped if the limit is reached. -1 disables the limit

    bool m_firstLine; //for the csv writer to detect wether the header must be written

    unsigned int m_NumberOfRecordedFiles; ///< necessary for the naming of the file if there is more than one start-stop cycle

    mitk::RealTimeClock::Pointer m_SystemTimeClock;  ///< system time clock for system time tag in output xml file

    bool m_DoNotOverwriteFiles; ///< do not overwrite any files if true

    std::map<const mitk::NavigationData*, std::pair<std::string, std::string> > m_AdditionalAttributes;
  };
}
#endif // #define _MITK_POINT_SET_SOURCE_H
