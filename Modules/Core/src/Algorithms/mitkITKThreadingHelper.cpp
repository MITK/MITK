/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cstdlib>
#include <thread>

#include <mitkExceptionMacro.h>
#include <mitkITKThreadingHelper.h>

#include <itkMultiThreaderBase.h>
#include <itkProcessObject.h>

#ifdef ITK_USE_TBB
#include <itkTBBMultiThreader.h>
#endif

void mitk::UseAllLogicalProcessors(itk::ProcessObject* filter)
{
  if (nullptr == filter)
  {
    mitkThrow() << "Cannot configure filter threading. Passed filter is null.";
  }

#ifdef ITK_USE_TBB
  if (nullptr == std::getenv("ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"))
  {
    auto threader = itk::TBBMultiThreader::New();
    threader->SetMaximumNumberOfThreads(
      std::max<itk::ThreadIdType>(itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads(),
                                  std::thread::hardware_concurrency()));
    filter->SetMultiThreader(threader);
  }
#endif
}
