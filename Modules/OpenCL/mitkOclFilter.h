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


#ifndef __mitkOclFilter_h
#define __mitkOclFilter_h

#include "mitkOclUtils.h"
#include "mitkCommon.h"

#include <MitkOclExports.h>

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

namespace mitk
{
/**
   @class OclFilter
   @brief Superclass for all OpenCL based filter

   This class takes care of loading and compiling the external GPU program code
  */
class MitkOcl_EXPORT OclFilter
{
public:
  /**
    * @brief Set the source file of the OpenCL shader
    *
    *  @param _filename Path to the file
    */
  void SetSourceFile(const char*);

  /**
    * @brief Set the folder of the OpenCL files
    *
    *  @param _path to the modulefolder that contains the gpuSource
    */
  void SetSourcePath(const char*);

  /**
    * @brief Set specific compilerflags to compile the CL source. Default is set to NULL;
    *        example: "-cl-fast-relaxed-math -cl-mad-enable -cl-strict-aliasing"
    *
    *  @param _flags to the modulefolder that contains the gpuSource
    */
  void SetCompilerFlags(const char*);

  /**
      @brief Returns true if the initialization was successfull
      */
  virtual bool IsInitialized();

  /** @brief Destructor */
  virtual ~OclFilter();

protected:
  /** @brief Constructor */
  OclFilter();

  /** @brief Constructor ( overloaded ) */
  OclFilter(const char* m_clFile);

  /** @brief Path to the *.cl source file */
  std::string m_clFile;

  /** @brief The source code to be compiled for the GPU */
  const char* m_clSource;

  /** @brief The path of the module folder in wich the gpu sourcefiles are stored */
  const char* m_clSourcePath;

  /** @brief String that contains the compiler flags */
  const char* m_clCompilerFlags;

  /** @brief The compiled OpenCL program */
  cl_program m_clProgram;

  /** @brief Command queue for the filter */
  cl_command_queue m_commandQue;

  /** @brief Unique ID of the filter, needs to be specified in the constructor of the derived class */
  std::string m_FilterID;

  /** @brief Set the working size for the following OpenCL kernel call */
  void SetWorkingSize( unsigned int locx, unsigned int locy,
                       unsigned int dimx, unsigned int dimy);

  /** @brief The local work size fo the filter */
  size_t m_localWorkSize[3];

  /** @brief The global work size of the filter */
  size_t m_globalWorkSize[3];

  /** @brief Execute the given kernel on the OpenCL Index-Space defined by the local and global work sizes
      */
  bool ExecuteKernel( cl_kernel kernel, unsigned int workSizeDim );

  /**
      * \brief Initialize all necessary parts of the filter
      *
      * The Initialize() method creates the command queue and the m_clProgram.
      * The program is either compiled from the given source or taken from the
      * OclResourceManager if the program was compiled already.
      */
  bool Initialize();

  /**
    * @brief Compile the program source
    *
    * @param preambel e.g. defines for the shader code
    */
  void CompileSource(const char*);

  /**
      * @brief Add some source code on the beginning of the loaded source
      *
      * In this way, some preprocessor flags for the CL compiler can at the beginning of the filter
      */
  void SetSourcePreambel( const char* );

  /*! @brief source preambel for e.g. #define commands to be inserted into the OpenCL source */
  const char* m_preambel;

  /** @brief  status of the filter */
  bool m_Initialized;

};
}
#endif // __mitkOclFilter_h
