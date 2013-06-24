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

#ifndef QmitkMorphologicalOperationsWidget_h
#define QmitkMorphologicalOperationsWidget_h

#include "../QmitkSegmentationUtilityWidget.h"
#include <ui_QmitkMorphologicalOperationsWidgetControls.h>


/**
\brief GUI class for Morphological Segmentation Tools.

*/
class QmitkMorphologicalOperationsWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkMorphologicalOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);
  ~QmitkMorphologicalOperationsWidget();



  public slots:
  void OnbtnClosingClicked();
  void OnbtnOpeningClicked();
  void OnbtnDilatationClicked();
  void OnbtnErosionClicked();
  void OnbtnFillHolesClicked();
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnRadioButtonsClicked();

protected:
   void enableButtons();
   void disableButtons();

private:
  Ui::QmitkMorphologicalOperationsWidgetControls m_Controls;
};

#endif
