/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrashDumpSessionOptionsRecord_h
#define mitkCrashDumpSessionOptionsRecord_h

#include <mitkCrashDumpSessionOptions.h>

namespace mitk
{
  /** Records what GetCrashDumpSessionOptions() reports; for BaseApplication. */
  void RecordCrashDumpSessionOptions(const CrashDumpSessionOptions& options);
}

#endif
