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

#ifndef __mitkOclResourceService_h
#define __mitkOclResourceService_h

#include <mitkServiceInterface.h>

#include <mitkOpenCL.h>

/**
 * @brief Declaration of the OpenCL Resources micro-service
 *
 * The OclResourceService defines an service interface for providing access to the
 * essential OpenCL-related variables. In addition the service can also store compiled
 * OpenCL Programs in order to avoid multiple compiling of a single program source
 */
class OclResourceService
{
public:
  /** @brief Returns a valid OpenCL Context (if applicable) or NULL if none present */
  virtual cl_context GetContext() const = 0;

  /** @brief Returns a valid cl_command_queue related to the (one) OpenCL context */
  virtual cl_command_queue GetCommandQueue() const = 0;

  /** @brief Returns the identifier of an OpenCL device related to the current context */
  virtual cl_device_id GetCurrentDevice() const = 0;

  /** @brief Checks if an OpenCL image format passed in is supported on current device */
  virtual bool GetIsFormatSupported( cl_image_format* format ) = 0;

  /** @brief Puts the OpenCL Context info in std::cout */
  virtual void PrintContextInfo() const = 0;

  /** @brief Insert program into the internal program storage
   *
   * @param program A cl_program object.
   * @param string Text identifier of the inserted program. Used for getting the program.
   * @param todo: what is the flag?
  */
  virtual void InsertProgram(cl_program program, std::string string, bool flag) = 0;

  /** @brief Get the cl_program by name
   * @param name Text identifier of the program.
   * @throws an mitk::Exception in case the program cannot be found
   */
  virtual cl_program GetProgram(const std::string& name) = 0;

  /** @brief Remove all invalid (=do not compile) programs from the internal storage */
  virtual void InvalidateStorage() = 0;

  /** @brief Remove given program from storage
   * @param name Text identifier of the program.
   */
  virtual void RemoveProgram(const std::string& name) = 0;

  /** @brief Get the maximum size of an image
   *
   * @param dimension (unsigned int) identifier of the image diemsion in {0,1,2}
   * @param image object type, either CL_MEM_OBJECT_IMAGE2D, CL_MEM_OBJECT_IMAGE3D
   */
  virtual unsigned int GetMaximumImageSize( unsigned int , cl_mem_object_type) = 0;

  virtual ~OclResourceService() = 0;

};

MITK_DECLARE_SERVICE_INTERFACE(OclResourceService, "OpenCLResourceService/1.0")
#endif // __mitkOclResourceService_h
