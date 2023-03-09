/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkOclFilter_h
#define mitkOclFilter_h

#include "mitkOclUtils.h"
#include "mitkCommon.h"

#include <vector>
#include <usModule.h>

#include <MitkOpenCLExports.h>

namespace mitk
{
/**
   @class OclFilter
   @brief Superclass for all OpenCL based filter.

   This class takes care of loading and compiling the external GPU program code.
  */
class MITKOPENCL_EXPORT OclFilter
{
public:
  /**
    * @brief Add a source file from the resource files to the
    *        OpenCL shader file list. Multiple files can be added to the list.
    *
    *  @param filename of the file in the resource system
    */
    void AddSourceFile(const char* filename);

  /**
    * @brief Set specific compilerflags to compile the CL source. Default is set to nullptr;
    *        example: "-cl-fast-relaxed-math -cl-mad-enable -cl-strict-aliasing"
    *
    *  @param flags to the modulefolder that contains the gpuSource
    */
  void SetCompilerFlags(const char* flags);

  /**
    * @brief Returns true if the initialization was successfull
    */
  virtual bool IsInitialized();

  /**
    * @brief Returns the amount of global memory of the used device in bytes
    */
  virtual unsigned long GetDeviceMemory();

  /** @brief Destructor */
  virtual ~OclFilter();

protected:

  typedef std::vector<const char*> CStringList;
  typedef std::vector<size_t> ClSizeList;

  /** @brief Constructor */
  OclFilter();

  /** @brief Constructor ( overloaded ) */
  OclFilter(const char* filename);

  /** @brief String that contains the compiler flags */
  const char* m_ClCompilerFlags;

  /** @brief The compiled OpenCL program */
  cl_program m_ClProgram;

  /** @brief Command queue for the filter */
  cl_command_queue m_CommandQue;

  /** @brief Unique ID of the filter, needs to be specified in the constructor of the derived class */
  std::string m_FilterID;

  /*! @brief source preambel for e.g. \c \#define commands to be inserted into the OpenCL source */
  const char* m_Preambel;

  /** @brief List of sourcefiles that will be compiled for this filter.*/
  CStringList m_ClFiles;

  /** @brief  status of the filter */
  bool m_Initialized;

  /** @brief The local work size fo the filter */
  size_t m_LocalWorkSize[3];

  /** @brief The global work size of the filter */
  size_t m_GlobalWorkSize[3];

  /** @brief Set the working size for the following OpenCL kernel call */
  void SetWorkingSize(unsigned int locx, unsigned int dimx,
                      unsigned int locy = 1, unsigned int dimy = 1,
                      unsigned int locz = 1, unsigned int dimz = 1);

  /** @brief Execute the given kernel on the OpenCL Index-Space defined by the local and global work sizes
      */
  bool ExecuteKernel( cl_kernel kernel, unsigned int workSizeDim );

  /** @brief Execute the given kernel on the OpenCL Index-Space defined by the local and global work sizes, but divide it into chunks of dimension chunksDim
      */
  bool ExecuteKernelChunks( cl_kernel kernel, unsigned int workSizeDim, size_t* chunksDim );

  /** @brief Execute the given kernel on the OpenCL Index-Space defined by the local and global work sizes, but divide it into chunks of dimension chunksDim and wait between
    * batches of batchSize chunks a time of waitTimems milliseconds
  */
  bool ExecuteKernelChunksInBatches(cl_kernel kernel, unsigned int workSizeDim, size_t* chunksDim, size_t batchSize, int waitTimems);
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
    */
  void CompileSource();

  /**
      * @brief Add some source code on the beginning of the loaded source
      *
      * In this way, some preprocessor flags for the CL compiler can at the beginning of the filter
      * @param preambel Source preambel for e.g. \c \#define commands to be inserted into the OpenCL source
      */
  void SetSourcePreambel(const char* preambel);

  /**
   * @brief Get the Module of the filter. Needs to be implemented by every subclass.
   *        The filter will load the OpenCL sourcefiles from this module context.
   */
  virtual us::Module* GetModule() = 0;

  /**
   * @brief Helper functions that load sourcefiles from the module context in the Initialize function.
   * @param SourceCodeList holds the sourcecode for every file as string, the SourceCodeSizeList holst the
   *        size of every file in bytes.
   * @param SourceCodeSizeList
   */
  void LoadSourceFiles(CStringList &SourceCodeList, ClSizeList &SourceCodeSizeList);
};
}
#endif
