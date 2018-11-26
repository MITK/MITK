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
#ifndef __mitkCESTImageDetectionHelper_h
#define __mitkCESTImageDetectionHelper_h

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
