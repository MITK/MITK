/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKCLARONTOOL_H_HEADER_INCLUDED_
#define MITKCLARONTOOL_H_HEADER_INCLUDED_

#include <mitkClaronInterface.h>
#include <mitkInternalTrackingTool.h>
#include <itkFastMutexLock.h>

namespace mitk
{
  /** Documentation:
  *   @brief  An object of this class represents a MicronTracker 2 tool.
  *           A tool has to be added to a tracking device which will then
  *           continiusely update the tool coordinates.
  */
  class ClaronTool : public InternalTrackingTool
  {
  public:

    mitkClassMacro(ClaronTool, InternalTrackingTool);
    itkNewMacro(Self);

    /**
    * @brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(const char* filename);
    /**
    * @brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(std::string filename);  

    const char* GetFile();
   
    /**
    * @brief Sets the handle of the tool.
    * @param handle The new handle of the tool.
    */
    void SetToolHandle (claronToolHandle handle);

    /**
    * @return Returns the calibration name which is used to identify the tool.
    */
    const char* GetCalibrationName();

    /**
    * @brief Sets the calibration name of the tool. Be careful, only use this method if you know what you are doing.
    * If you want to change the tool name use the method setToolName instead!
    */
    void SetCalibrationName(const char* name);

    /**
    * @return Returns the toolhandle of the tool.
    */
    claronToolHandle GetToolHandle();
   
  protected:

    ClaronTool();
    virtual ~ClaronTool();

    claronToolHandle m_ToolHandle;
    std::string m_CalibrationName;
    ClaronInterface* m_Device;
    std::string m_Filename;
  };
}//mitk
#endif // MITKCLARONTOOL_H_HEADER_INCLUDED_ 
