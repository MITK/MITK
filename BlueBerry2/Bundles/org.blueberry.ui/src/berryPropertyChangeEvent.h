/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYPROPERTYCHANGEEVENT_H_
#define BERRYPROPERTYCHANGEEVENT_H_

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "berryUiDll.h"

namespace berry {

using namespace osgi::framework;

/**
 * An event object describing a change to a named property.
 * <p>
 * This concrete class was designed to be instantiated, but may
 * also be subclassed if required.
 * </p>
 * <p>
 * The JFace frameworks contain classes that report property
 * change events for internal state changes that may be of interest
 * to external parties. A special listener interface
 * (<code>IPropertyChangeListener</code>) is defined for this purpose,
 * and a typical class allow listeners to be registered via
 * an <code>addPropertyChangeListener</code> method.
 * </p>
 *
 * @see IPropertyChangeListener
 */
class BERRY_UI PropertyChangeEvent : public Object {

public:

  osgiObjectMacro(PropertyChangeEvent);

private:

    /**
     * The name of the changed property.
     */
    std::string propertyName;

    /**
     * The old value of the changed property, or <code>null</code> if
     * not known or not relevant.
     */
    Object::Pointer oldValue;

    /**
     * The new value of the changed property, or <code>null</code> if
     * not known or not relevant.
     */
    Object::Pointer newValue;

    /**
     * The object on which the property change occured
     */
    Object::Pointer source;

public:

    /**
     * Creates a new property change event.
     *
     * @param source the object whose property has changed
     * @param property the property that has changed (must not be <code>null</code>)
     * @param oldValue the old value of the property, or <code>null</code> if none
     * @param newValue the new value of the property, or <code>null</code> if none
     */
    PropertyChangeEvent(Object::Pointer source, const std::string& property, Object::Pointer oldValue,
            Object::Pointer newValue);

    /**
     * Returns the new value of the property.
     *
     * @return the new value, or <code>null</code> if not known
     *  or not relevant (for instance if the property was removed).
     */
    Object::Pointer GetNewValue();

    /**
     * Returns the old value of the property.
     *
     * @return the old value, or <code>null</code> if not known
     *  or not relevant (for instance if the property was just
     *  added and there was no old value).
     */
    Object::Pointer GetOldValue();

    /**
     * Returns the name of the property that changed.
     * <p>
     * Warning: there is no guarantee that the property name returned
     * is a constant string.  Callers must compare property names using
     * equals, not ==.
     * </p>
     *
     * @return the name of the property that changed
     */
    std::string GetProperty();

    /**
     * Returns the object whose property has changed
     *
     * @return the object whose property has changed
     */
    Object::Pointer GetSource();

};

}

#endif /* BERRYPROPERTYCHANGEEVENT_H_ */
