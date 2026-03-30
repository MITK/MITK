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

#include <mitkNodePredicateBase.h>

namespace mitk
{
  class Image;

  /**
  \brief Check whether an image is any type of CEST image (CEST/WASABI or T1).

  An image is considered a CEST image if it is either a CEST/WASABI image
  (has the "CEST.Offsets" property) or a CEST T1 image (has the "CEST.TREC" property).

  \param[in] cestImage Pointer to the image to check. May be nullptr.
  \return True if the image is any type of CEST image, false otherwise (including nullptr).
  \sa IsCESTorWasabiImage, IsCESTT1Image, CESTImageNormalizationFilter
  */
  MITKCEST_EXPORT bool IsAnyCESTImage(const Image* cestImage);

  /**
  \brief Check whether an image is a CEST or WASABI image.

  Detection is based on the presence of the "CEST.Offsets" property on the image.
  This property is typically set by the CustomTagParser when loading DICOM data from
  a CEST or WASABI sequence.

  \param[in] cestImage Pointer to the image to check. May be nullptr.
  \return True if the image has CEST offset information, false otherwise (including nullptr).
  \sa IsAnyCESTImage, IsCESTT1Image, CustomTagParser
  */
  MITKCEST_EXPORT bool IsCESTorWasabiImage(const Image* cestImage);

  /**
  \brief Check whether an image is a CEST T1 image.

  Detection is based on the presence of the "CEST.TREC" property on the image.
  This property is typically set by the CustomTagParser when loading DICOM data from
  a T1 recovery sequence used for CEST analysis.

  \param[in] cestImage Pointer to the image to check. May be nullptr.
  \return True if the image has CEST T1 recovery time information, false otherwise (including nullptr).
  \sa IsAnyCESTImage, IsCESTorWasabiImage, CustomTagParser
  */
  MITKCEST_EXPORT bool IsCESTT1Image(const Image* cestImage);

  /**
  \brief Create a node predicate that matches any CEST image node.

  Creates a NodePredicateFunction that checks whether a DataNode contains
  an image that satisfies IsAnyCESTImage().

  \return A NodePredicateBase smart pointer for use in DataStorage queries.
  \sa IsAnyCESTImage, CreateCESTorWasabiImageNodePredicate, CreateCESTT1ImageNodePredicate
  */
  MITKCEST_EXPORT NodePredicateBase::Pointer CreateAnyCESTImageNodePredicate();

  /**
  \brief Create a node predicate that matches CEST or WASABI image nodes.

  Creates a NodePredicateFunction that checks whether a DataNode contains
  an image that satisfies IsCESTorWasabiImage().

  \return A NodePredicateBase smart pointer for use in DataStorage queries.
  \sa IsCESTorWasabiImage, CreateAnyCESTImageNodePredicate, CreateCESTT1ImageNodePredicate
  */
  MITKCEST_EXPORT NodePredicateBase::Pointer CreateCESTorWasabiImageNodePredicate();

  /**
  \brief Create a node predicate that matches CEST T1 image nodes.

  Creates a NodePredicateFunction that checks whether a DataNode contains
  an image that satisfies IsCESTT1Image().

  \return A NodePredicateBase smart pointer for use in DataStorage queries.
  \sa IsCESTT1Image, CreateAnyCESTImageNodePredicate, CreateCESTorWasabiImageNodePredicate
  */
  MITKCEST_EXPORT NodePredicateBase::Pointer CreateCESTT1ImageNodePredicate();

} // END mitk namespace

#endif
