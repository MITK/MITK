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

class QWidget;

/**
* \brief Saves the label information of a segmentation as a preset file.
*
* Triggers a file dialog to specify the location where to store the preset.
* \pre segmentation must be a valid pointer.
* \param parent Parent of the file dialog and the error message.
* \param segmentation Pointer to the segmentation that serves as template for the preset.
*/
void MITKSEGMENTATIONUI_EXPORT QmitkSaveMultiLabelPreset(QWidget* parent, const mitk::MultiLabelSegmentation* segmentation);

/**
* \brief Loads a label preset and imposes it on all passed segmentations.
*
* Triggers a file dialog to specify the location where to load the preset.
* \param parent Parent of the file dialog.
* \param segmentations Vector of pointers to the segmentations that should be modified according to the preset.
*        Invalid segmentations (nullptr) will be ignored.
*/
void MITKSEGMENTATIONUI_EXPORT QmitkLoadMultiLabelPreset(QWidget* parent, const std::vector<mitk::MultiLabelSegmentation::Pointer>& segmentations);

#endif
