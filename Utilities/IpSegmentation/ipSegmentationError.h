/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef IPSEGMENTATION_ERROR_H
#define IPSEGMENTATION_ERROR_H

enum {
  ipMITKSegmentationOK = 0     /*! No errors at all. */,
  ipMITKSegmentationOUT_OF_MEMORY  /*! Not enough memory available. */,
  ipMITKSegmentationWRONG_TYPE  /*! Segmentation image has a wrong type. */,
  ipMITKSegmentationUNDO_DISABLED  /*! Undo cannot be performed. */,
  ipMITKSegmentationPIC_NULL    /*! PIC descriptor is null. */,
  ipMITKSegmentationUNKNOWN_ERROR  /*! Some unknown error occurred. */
};

/*!
\brief Prints an error message and exits
if an error has occurred.
@param error error code
*/
void ipMITKSegmentationError (int error);

/*!
\brief Checks the segmentation image.
@param segmentation the segmentation image.
*/
void ipMITKSegmentationCheck (mitkIpPicDescriptor* segmentation);

#endif
