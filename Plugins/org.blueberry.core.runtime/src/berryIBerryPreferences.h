/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef BERRYIBERRYPREFERENCES_H_
#define BERRYIBERRYPREFERENCES_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryIPreferences.h"
#include "berryMessage.h"


namespace berry
{

  /**
   * Like IEclipsePreferences an extension to the osgi-IPreferences
   * to send out events when nodes or values changed in a node.
   */
  struct org_blueberry_core_runtime_EXPORT IBerryPreferences : virtual public IPreferences
  {
    berryObjectMacro(berry::IBerryPreferences)

    class org_blueberry_core_runtime_EXPORT ChangeEvent {
      IBerryPreferences* const m_Source;
      const QString m_Property;
      const QString m_OldValue;
      const QString m_NewValue;

      public:

      ChangeEvent(IBerryPreferences* source, const QString& property,
                  const QString& oldValue, const QString& newValue);

      IBerryPreferences* GetSource() const;
      QString GetProperty() const;
      QString GetOldValue() const;
      QString GetNewValue() const;
    };

    ~IBerryPreferences() override;

    /**
     * Invoked when this node was changed, that is when a property
     * was changed or when a new child node was inserted.
     */
    berry::Message1<const IBerryPreferences*> OnChanged;

    /**
     * Adds a property change listener to this preferences node.
     * The listener will be informed about property changes of
     * this preference node, but not about changes to child nodes.
     */
    berry::Message1<const ChangeEvent&> OnPropertyChanged;
  };

}  // namespace berry

#endif /*BERRYIBERRYPREFERENCES_H_*/
