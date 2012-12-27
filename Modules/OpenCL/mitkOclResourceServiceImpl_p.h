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

#ifndef __mitkOclResourceServiceImpl_h
#define __mitkOclResourceServiceImpl_h

#include <map>

//Micro Services
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>

//ocl
#include "mitkOclResourceService.h"
#include "mitkOclUtils.h"
#include "mitkOclImageFormats.h"

//todo add docu!

class OclResourceServiceImpl
    : public US_BASECLASS_NAME, public OclResourceService
{
public:
  typedef std::map< std::string, cl_program > ProgramMapType;

  OclResourceServiceImpl();

  ~OclResourceServiceImpl();

  cl_context GetContext() const;

  cl_command_queue GetCommandQueue() const;

  cl_device_id GetCurrentDevice() const;

  bool GetIsFormatSupported(cl_image_format *);

  void PrintContextInfo();

  void InsertProgram(cl_program _program_in, std::string name, bool forceOverride=true);

  cl_program GetProgram(const std::string&name) const;

  void InvalidateStorage();

  void RemoveProgram(const std::string&name);

  unsigned int GetMaximumImageSize(unsigned int dimension, cl_mem_object_type _imagetype);

private:
  void CreateContext();

  /** The context */
  cl_context m_Context;

  /** Available OpenCL devices */
  cl_device_id* m_Devices;

  /** Class for handling (un)supported GPU image formats **/
  mitk::OclImageFormats::Pointer m_ImageFormats;

  /** The command queue*/
  cl_command_queue m_CommandQueue;

  /** Map containing all available (allready compiled) OpenCL Programs */
  ProgramMapType m_ProgramStorage;

};

#endif // __mitkOclResourceServiceImpl_h
