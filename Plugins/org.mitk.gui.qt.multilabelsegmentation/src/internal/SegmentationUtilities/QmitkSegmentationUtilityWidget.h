/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
 * The time navigation controller is not not available at all times and hence this method can return nullptr.
 */
class QmitkSegmentationUtilityWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationUtilityWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent = nullptr);
  ~QmitkSegmentationUtilityWidget() override;

  /** \brief Usually called only from QmitkSegmentationUtilitiesView::RenderWindowPartActivated() and QmitkSegmentationUtilitiesView::RenderWindowPartDeactivated().
   */
  void SetTimeNavigationController(mitk::SliceNavigationController* timeNavigationController);

protected:
  /** \brief Call this method to access the time navigation controller.
   *
   * \return Pointer to the time navigation controller or nullptr, if it is not available.
   */
  mitk::SliceNavigationController* GetTimeNavigationController() const;

private:
  mitk::SliceNavigationController* m_TimeNavigationController;
};

#endif
