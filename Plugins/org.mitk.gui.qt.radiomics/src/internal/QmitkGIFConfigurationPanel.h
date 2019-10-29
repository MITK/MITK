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

#ifndef QMITK_GIF_CONFIGURATION_PANEL_H
#define QMITK_GIF_CONFIGURATION_PANEL_H

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

#endif // QMITK_GIF_CONFIGURATION_PANEL_H