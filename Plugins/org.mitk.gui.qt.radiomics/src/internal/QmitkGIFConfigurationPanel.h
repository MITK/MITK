/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkGIFConfigurationPanel_h
#define QmitkGIFConfigurationPanel_h

// MITK
#include <mitkAbstractGlobalImageFeature.h>

// QT
#include <qwidget.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

namespace mitkUI
{
  class GIFConfigurationPanel : public QWidget
  {
    Q_OBJECT
  public:
    GIFConfigurationPanel(QWidget * parent, mitk::AbstractGlobalImageFeature::Pointer calculator);
    void CalculateFeaturesUsingParameters(const mitk::Image::Pointer & feature, const mitk::Image::Pointer &mask, std::map < std::string, us::Any> parameter, mitk::AbstractGlobalImageFeature::FeatureListType &featureList);

  protected slots:
    void OnSButtonShowAdditionalConfigurationPressed(bool status);

  private:
    mitk::AbstractGlobalImageFeature::Pointer m_FeatureCalculator;

    QCheckBox* m_CheckCalculateFeature;
    QPushButton* m_ButtonShowAdditionalConfiguration;
    QGroupBox* m_GroupBoxArguments;
  };
}

#endif
