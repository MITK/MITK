/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkFileSystem.h
 * \brief Portability header that provides a unified namespace alias for the C++ filesystem library.
 *
 * This header detects whether \c \<filesystem\> or \c \<experimental/filesystem\> is available
 * and introduces the namespace alias \c fs that refers to the appropriate implementation.
 * If the standard \c \<filesystem\> header is found, the preprocessor symbol \c MITK_HAS_FILESYSTEM
 * is also defined.
 *
 * \note Prefer including this header instead of \c \<filesystem\> directly to ensure
 *       compatibility with older compilers that only provide the experimental variant.
 *
 * \ingroup Core
 */

#ifndef mitkFileSystem_h
#define mitkFileSystem_h

#if __has_include(<filesystem>)
  #define MITK_HAS_FILESYSTEM
  #include <filesystem>
  /** \brief Namespace alias referring to \c std::filesystem. */
  namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
  #include <experimental/filesystem>
  /** \brief Namespace alias referring to \c std::experimental::filesystem. */
  namespace fs = std::experimental::filesystem;
#endif

#endif
