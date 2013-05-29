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

#ifndef QmitkSegmentationUtilityWidget_h
#define QmitkSegmentationUtilityWidget_h

#include <QWidget>

namespace mitk
{
  class SliceNavigationController;
}

class QmitkSegmentationUtilityWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationUtilityWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = NULL);
  virtual ~QmitkSegmentationUtilityWidget();

  void SetTimeNavigationController(mitk::SliceNavigationController* timeNavigationController);

protected:
  mitk::SliceNavigationController* GetTimeNavigationController() const;

private:
  mitk::SliceNavigationController* m_TimeNavigationController;
};

#endif
