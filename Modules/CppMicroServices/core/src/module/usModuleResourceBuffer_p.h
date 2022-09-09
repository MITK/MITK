/*============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center (DKFZ)
  All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

============================================================================*/

#ifndef USMODULERESOURCEBUFFER_P_H
#define USMODULERESOURCEBUFFER_P_H

#include <usCoreExport.h>

#include <streambuf>

US_BEGIN_NAMESPACE

class ModuleResourceBufferPrivate;

class US_Core_EXPORT ModuleResourceBuffer: public std::streambuf
{

public:

  explicit ModuleResourceBuffer(void* data, std::size_t size,
                                std::ios_base::openmode mode);

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

US_END_NAMESPACE

#endif // USMODULERESOURCEBUFFER_P_H
