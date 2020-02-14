/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPREFERENCEPAGEPARAMETERVALUES_H
#define BERRYPREFERENCEPAGEPARAMETERVALUES_H

#include <berryIParameterValues.h>
#include <berryIRegistryEventListener.h>

namespace berry {

struct IPreferences;

/**
 * <p>
 * Provides the parameter values for the show preferences command.
 * </p>
 * <p>
 * To disambiguate preference pages with the same local label, names are
 * constructed incorporating the full path of preference page labels. For
 * instance names like <code>General > Appearance</code> and
 * <code>Java > Appearance</code> avoid the problem of trying to put two
 * <code>Appearance</code> keys into the parameter values map.
 * </p>
 * <p>
 * This is only intended for use within the
 * <code>org.blueberry.ui.qt.workbench</code> plug-in.
 * </p>
 */
class PreferencePageParameterValues : public QObject, public IParameterValues, private IRegistryEventListener
{
  Q_OBJECT

public:

  PreferencePageParameterValues();

  QHash<QString,QString> GetParameterValues() const override;


private:

  mutable QHash<QString,QString> preferenceMap;

  void Added(const QList<SmartPointer<IExtension> >& extensions) override;
  void Removed(const QList<SmartPointer<IExtension> >& extensions) override;
  void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;
  void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;

};

}

#endif // BERRYPREFERENCEPAGEPARAMETERVALUES_H
