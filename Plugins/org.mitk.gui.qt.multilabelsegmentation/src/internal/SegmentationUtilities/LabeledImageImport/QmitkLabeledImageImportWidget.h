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

#ifndef QmitkLabeledImageImportWidget_h
#define QmitkLabeledImageImportWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkLabeledImageImportWidgetControls.h>

/*!
  \brief QmitkLabeledImageImportWidget

  Tool masks an image with a binary image or a surface. The Method requires
  an image and a binary image mask or a surface. The input image and the binary
  image mask must be of the same size. Masking with a surface creates first a
  binary image of the surface and then use this for the masking of the input image.
*/
class QmitkLabeledImageImportWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkLabeledImageImportWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);

  /** @brief Defaul destructor. */
  ~QmitkLabeledImageImportWidget();

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);

  /** @brief This slot is called if user activates the button to mask an image. */
  void OnImportPressed();

private:

  /** @brief Check if selections is valid. */
  void SelectionControl( unsigned int index, const mitk::DataNode* selection);

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  Ui::QmitkLabeledImageImportWidgetControls m_Controls;
};

#endif
