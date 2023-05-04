/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelPresetHelper_h
#define QmitkMultiLabelPresetHelper_h

#include <mitkLabelSetImage.h>

#include <MitkSegmentationUIExports.h>

/**@brief Helper function to save the label information of a passed segmentation as preset.
*
* Helper function triggers a file dialog to specify the location where to store the preset.
* @pre segmentation must be a valid pointer.
* @param segmentation pointer to the segmentation that serves as template for the preset.
*/
void MITKSEGMENTATIONUI_EXPORT QmitkSaveMultiLabelPreset(const mitk::LabelSetImage* segmentation);

/**@brief Helper function to loads a label preset and imposes it on all passed segmentations.
*
* Helper function triggers a file dialog to specify the location where to load the preset.
* @param segmentations vector of pointers to the segmentations that should be modified according to the preset.
* Invalid segmentations (nullptr) will be ignored.
*/
void MITKSEGMENTATIONUI_EXPORT QmitkLoadMultiLabelPreset(const std::vector<mitk::LabelSetImage::Pointer>& segmentations);

#endif
