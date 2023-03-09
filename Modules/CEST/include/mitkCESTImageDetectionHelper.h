/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkCESTImageDetectionHelper_h
#define mitkCESTImageDetectionHelper_h

#include <MitkCESTExports.h>

#include "mitkNodePredicateBase.h"

namespace mitk
{
  class Image;

  /** This helper function can be used to check if an image is an type of CEST image.
  */
  MITKCEST_EXPORT bool IsAnyCESTImage(const Image* cestImage);

  /** This helper function can be used to check if an image is an CEST image (recording a CEST, Wasabi or simelar sequence).
  */
  MITKCEST_EXPORT bool IsCESTorWasabiImage(const Image* cestImage);

  /** This helper function can be used to check if an image is an CEST T1 image (recording a T1 sequence for CEST contrasts).
  */
  MITKCEST_EXPORT bool IsCESTT1Image(const Image* cestImage);

  MITKCEST_EXPORT NodePredicateBase::Pointer CreateAnyCESTImageNodePredicate();
  MITKCEST_EXPORT NodePredicateBase::Pointer CreateCESTorWasabiImageNodePredicate();
  MITKCEST_EXPORT NodePredicateBase::Pointer CreateCESTT1ImageNodePredicate();

} // END mitk namespace

#endif
