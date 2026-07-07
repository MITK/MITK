/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULERESOURCEBUFFER_P_H
#define USMODULERESOURCEBUFFER_P_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <streambuf>

namespace us {

class ModuleResourceBufferPrivate;

/** \brief Stream buffer for reading module resource data.
 *
 * This is a private implementation class used by ModuleResourceStream.
 */
class MITKCPPMICROSERVICES_EXPORT ModuleResourceBuffer: public std::streambuf
{

public:

  /** \brief Construct a ModuleResourceBuffer.
   * \param[in] data Pointer to the raw resource data.
   * \param[in] size Size of the resource data in bytes.
   * \param[in] mode The open mode for the stream buffer.
   */
  explicit ModuleResourceBuffer(void* data, std::size_t size,
                                std::ios_base::openmode mode);

  /** \brief Destructor. */
  ~ModuleResourceBuffer() override;

private:

  int_type underflow() override;

  int_type uflow() override;

  int_type pbackfail(int_type ch) override;

  std::streamsize showmanyc() override;

  pos_type seekoff (off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
  pos_type seekpos (pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;

  // purposely not implemented
  ModuleResourceBuffer(const ModuleResourceBuffer&);
  ModuleResourceBuffer& operator=(const ModuleResourceBuffer&);

private:

  ModuleResourceBufferPrivate* d;

};

}

#endif // USMODULERESOURCEBUFFER_P_H
