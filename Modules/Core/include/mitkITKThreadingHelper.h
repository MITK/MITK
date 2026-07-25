/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkITKThreadingHelper_h
#define mitkITKThreadingHelper_h

#include <MitkCoreExports.h>

namespace itk
{
  class ProcessObject;
}

namespace mitk
{
  /**
   * \brief Lets the given filter use all logical processors of the machine.
   *
   * Compute-bound, long-running filters like pixel-based model fitting profit
   * from all logical processors, while the global ITK thread default is capped
   * in favor of interactive workloads (see MitkCoreActivator). This helper
   * equips the filter with a dedicated TBB-based multi threader raised to the
   * number of logical processors: the increased concurrency is scoped to the
   * filter's Update() calls and TBB's work stealing balances strongly varying
   * per-element cost.
   *
   * The function is a no-op if the user configured ITK threading explicitly
   * via the ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS environment variable or if
   * ITK was built without TBB support.
   *
   * \param[in,out] filter The filter that should use all logical processors.
   * \throw mitk::Exception If filter is null.
   */
  MITKCORE_EXPORT void UseAllLogicalProcessors(itk::ProcessObject* filter);
}

#endif
