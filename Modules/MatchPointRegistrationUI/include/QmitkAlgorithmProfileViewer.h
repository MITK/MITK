/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAlgorithmProfileViewer_h
#define QmitkAlgorithmProfileViewer_h

#include <mapDeploymentDLLInfo.h>

#include <MitkMatchPointRegistrationUIExports.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkAlgorithmProfileViewer;
}

/**
 * \class QmitkAlgorithmProfileViewer
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmProfileViewer : public QWidget
{
  Q_OBJECT

public:
  QmitkAlgorithmProfileViewer(QWidget *parent = nullptr);
  ~QmitkAlgorithmProfileViewer() override;

  /**
   * \brief Updates the widget according to the new info.
   * \param newInfo pointer to the info instance.
   * \remark The DLLInfo is not stored internally or as reference
   * to update the widget you must use the updateInfo() method.
   */
  void updateInfo(const map::deployment::DLLInfo *newInfo);

public Q_SLOTS:
  /**
    * \brief Slot that can be used to trigger updateInfo();
    */
  void OnInfoChanged(const map::deployment::DLLInfo *newInfo);

protected:
  std::unique_ptr<Ui::QmitkAlgorithmProfileViewer> m_Controls;
};

#endif
