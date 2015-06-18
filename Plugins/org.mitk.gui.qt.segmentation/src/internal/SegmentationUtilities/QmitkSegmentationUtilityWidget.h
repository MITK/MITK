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

/** \brief Base class for segmentation utility widgets that need access to the time navigation controller.
 *
 * Call GetTimeNavigationController() in your derived class to gain access to the time navigation controller.
 * The time navigation controller is not not available at all times and hence this method can return NULL.
 */
class QmitkSegmentationUtilityWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationUtilityWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);
  virtual ~QmitkSegmentationUtilityWidget();

  /** \brief Usually called only from QmitkSegmentationUtilitiesView::RenderWindowPartActivated() and QmitkSegmentationUtilitiesView::RenderWindowPartDeactivated().
   */
  void SetTimeNavigationController(mitk::SliceNavigationController* timeNavigationController);

protected:
  /** \brief Call this method to access the time navigation controller.
   *
   * \return Pointer to the time navigation controller or NULL, if it is not available.
   */
  mitk::SliceNavigationController* GetTimeNavigationController() const;

private:
  mitk::SliceNavigationController* m_TimeNavigationController;
};

#endif
