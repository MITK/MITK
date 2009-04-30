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
  class ClaronTrackingDevice;
  /** Documentation:
  *   \brief  An object of this class represents a MicronTracker 2 tool.
  *           A tool has to be added to a tracking device which will then
  *           continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITK_IGT_EXPORT ClaronTool : public InternalTrackingTool
  {
  public:
    friend ClaronTrackingDevice;
    mitkClassMacro(ClaronTool, InternalTrackingTool);
    

    /**
    * \brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(const char* filename);
    /**
    * \brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(std::string filename);  

    std::string GetFile();
   
    /**
    * \brief Sets the handle of the tool.
    * \param handle The new handle of the tool.
    */
    void SetToolHandle (claronToolHandle handle);

    /**
    * \return Returns the calibration name which is used to identify the tool.
    */
    std::string GetCalibrationName();

    /**
    * \brief Sets the calibration name of the tool. Be careful, only use this method if you know what you are doing.
    * If you want to change the tool name use the method setToolName instead!
    */
    void SetCalibrationName(std::string name);

    /**
    * @return Returns the tool handle of the tool.
    */
    claronToolHandle GetToolHandle();
   
  protected:
    itkNewMacro(Self);
    ClaronTool();
    virtual ~ClaronTool();
    /** \brief Tool handle variable from tracking device */
    claronToolHandle m_ToolHandle;
    /** \brief  Variable which holds the Tool's calibration name */
    std::string m_CalibrationName;
    /** \brief Variable to check filename's format and to get back complete filename */
    std::string m_Filename;
  };
}//mitk
#endif // MITKCLARONTOOL_H_HEADER_INCLUDED_ 
