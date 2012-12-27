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

#ifndef __mitkOclUtils_h
#define __mitkOclUtils_h

#include "mitkOpenCL.h"

#include <string>
#include <MitkOclExports.h>

#define CHECK_OCL_ERR(_er) oclCheckError(_er, __FILE__, __LINE__);

/**
 @brief Method to estimate an integer quotient C from given dividend and divisor higher or equal to
        the corresponding floating quotient

  If the divisor is a factor of the dividend, the dividend/divisor is an integer value and is returned. If not,
  the nearest higher integer is returned. So it holds for the return value C that C * divisor is equal or greater then
  the dividend. In OpenCL context useful for estimating the local/global working dimension of a NDRange so that all
  image data is covered by the parallelisation scheme.
 */
MitkOcl_EXPORT unsigned int iDivUp(unsigned int dividend, unsigned int divisor);

/*! \brief Loads a program source code from a text-file to a string,

@param srcFilename: the file location
@param srcPreamble: will be added on the top (e.g. #define USE_UCHAR / USE_FLOAT)

@return a char*
*/
MitkOcl_EXPORT char* oclLoadProgramSource( const char* ,const char* ,size_t* );

/**
 @brief Returns the name of an OpenCL Error as a string

 Most of the OpenCL Methods ( cl<NAME> ) return an integer error code. This method translates the
 error value given as parameter to the corresponding error name. For example the value -30 will be translated
 to CL_INVALID_VALUE
 */
MitkOcl_EXPORT std::string GetOclErrorAsString( int _clErr );

/**
 @brief Checks whether the given value corresponds to an OpenCL Error value and prints this message out as MITK_ERROR if yes
 */
MitkOcl_EXPORT void GetOclError(int _clErr);

/**
 @brief Returns a platform ID of an OpenCL-capable GPU, or throws an exception
*/
MitkOcl_EXPORT cl_int oclGetPlatformID(cl_platform_id* selectedPlatform);

/*! \brief Prints out the essential support information about current device */
MitkOcl_EXPORT void oclPrintDeviceInfo(cl_device_id);

/*! @brief Prints the available memory info about the given object to std::cout
  */
MitkOcl_EXPORT void oclPrintMemObjectInfo( cl_mem memobj);

/*! \brief Checks the given code for errors and produces a std::cout output if
  the _err does not equal CL_SUCCESS. The output includes also the filename and the line
  number of the method call.
  */
MitkOcl_EXPORT bool oclCheckError(int _err, const char*, int);

/*! \brief Logs the GPU Program binary code

@param clProg: the OpenCL Program to log
@param clDev: the OpenCL-capable device the program was tried to be compiled for
*/
MitkOcl_EXPORT void oclLogBinary(cl_program clProg, cl_device_id clDev);

/*! \brief Shows the OpenCL-Program build info, called if clBuildProgram != CL_SUCCES

@param clProg: the OpenCL Program to log
@param clDev: the OpenCL-capable device the program was tried to be compiled for
*/
MitkOcl_EXPORT void oclLogBuildInfo(cl_program clProg, cl_device_id clDev);

/** \brief Print out all supported image formats for given image type

  @param _type the image type ( CL_MEM_OBJECT_2D or CL_MEM_OBJECT_3D )
  @param _context the OpenCL context to be examined
  */
MitkOcl_EXPORT void GetSupportedImageFormats(cl_context _context, cl_mem_object_type _type);

/**
 @brief Translates the internal image type identifier to a human readable description string
*/
MitkOcl_EXPORT std::string GetImageTypeAsString( const unsigned int _in);

#endif //mitkOclUtils_h
