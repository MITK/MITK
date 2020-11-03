/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSIONFACTORY_H
#define BERRYEXTENSIONFACTORY_H

#include <berryIConfigurationElement.h>
#include <berryIExecutableExtensionFactory.h>
#include <berryIExecutableExtension.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * Factory for the workbench's public extensions.
 * <p>
 * This allows the extensions to be made available for use by RCP applications
 * without exposing their concrete implementation classes.
 * </p>
 */
class BERRY_UI_QT ExtensionFactory : public QObject, public IExecutableExtensionFactory,
    public IExecutableExtension
{
  Q_OBJECT
  Q_INTERFACES(berry::IExecutableExtensionFactory berry::IExecutableExtension)

public:

  ~ExtensionFactory() override;

  /**
   * Factory ID for the Appearance preference page.
   */
  static const QString STYLE_PREFERENCE_PAGE; // = "stylePreferencePage";

  /**
   * Factory ID for the Perspectives preference page.
   */
  static const QString PERSPECTIVES_PREFERENCE_PAGE; // = "perspectivesPreferencePage";

  /*
   * Factory ID for the show in contribution.
   */
  //static const QString SHOW_IN_CONTRIBUTION = "showInContribution";


  /**
   * Creates the object referenced by the factory id obtained from the
   * extension data.
   */
  QObject* Create() override;

  /*
   * @see IExecutableExtension#SetInitializationData
   */
  void SetInitializationData(const SmartPointer<IConfigurationElement>& config,
                             const QString& propertyName, const Object::Pointer& data) override;

private:

  QObject* Configure(QObject* obj);

  IConfigurationElement::Pointer config;

  QString id;

  QString propertyName;

};

}

#endif // BERRYEXTENSIONFACTORY_H
