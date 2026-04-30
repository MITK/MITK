/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAntiAliasing_h
#define mitkAntiAliasing_h

namespace mitk
{
  /** \brief Anti-aliasing methods for rendering.
   * \ingroup Core
   */
  enum class AntiAliasing : int
  {
    None,              ///< No anti-aliasing.
    FastApproximate    ///< Fast approximate anti-aliasing (FXAA).
  };
}

#endif
