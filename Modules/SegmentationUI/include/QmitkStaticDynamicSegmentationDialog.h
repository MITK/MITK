/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStaticDynamicSegmentationDialog_h
#define QmitkStaticDynamicSegmentationDialog_h

#include <MitkSegmentationUIExports.h>

#include <QMessageBox>

#include <mitkImage.h>

/**
 * \brief Dialog for static / dynamic segmentation node creation.
 *
 * This dialog is used to ask a user about the dimensionality of a newly created segmentation.
 * If the user wants to create a static / 3D segmentation image from a given 4D reference image,
 * the time steps of the reference image are combined into a single time step, with a step duration
 * that is as long as the step duration of all 4D time steps combined.
 */
class MITKSEGMENTATIONUI_EXPORT QmitkStaticDynamicSegmentationDialog : public QMessageBox
{
  Q_OBJECT

public:

  QmitkStaticDynamicSegmentationDialog(QWidget* parent = nullptr);

  void SetReferenceImage(const mitk::Image* referenceImage);
  mitk::Image::ConstPointer GetSegmentationTemplate() const;
  
private Q_SLOTS:

  void OnStaticButtonClicked(bool checked = false);

private:

  mitk::Image::ConstPointer m_SegmentationTemplate;
  const mitk::Image* m_ReferenceImage;
};

#endif
