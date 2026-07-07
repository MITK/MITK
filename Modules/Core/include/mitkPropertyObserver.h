/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyObserver_h
#define mitkPropertyObserver_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <itkEventObject.h>

namespace mitk
{
  /**
    \brief Convenience class to observe changes of a mitk::BaseProperty.

    This class registers itself as an ITK observer to a BaseProperty and gets
    informed of changes to the property. Whenever such a change occurs, the virtual
    method PropertyChanged() or PropertyRemoved() is called. This way, derived
    classes can implement behaviour for more specific properties (e.g. ColorProperty)
    without the need to reimplement the Subject-Observer handling.

  */

  class BaseProperty;

  /**
   * \brief Base class for property observers providing callbacks for property changes and deletion.
   *
   * This class provides a guard mechanism (m_SelfCall) to prevent re-entrant notifications
   * when the observer itself modifies the property.
   *
   * \sa PropertyView
   * \sa PropertyEditor
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT PropertyObserver
  {
  public:
    /** \brief Constructor. */
    PropertyObserver();

    /** \brief Virtual destructor. */
    virtual ~PropertyObserver();

    /** \brief Called when the observed property value has changed. */
    virtual void PropertyChanged() = 0;

    /** \brief Called when the observed property is about to be deleted. */
    virtual void PropertyRemoved() = 0;

  protected:
    /** \brief Marks the beginning of a self-initiated property modification.
     *
     * Call this before modifying the observed property from within the observer
     * to suppress recursive notification callbacks.
     */
    void BeginModifyProperty();

    /** \brief Marks the end of a self-initiated property modification. */
    void EndModifyProperty();

    unsigned long m_ModifiedTag;  ///< ITK observer tag for the ModifiedEvent.
    unsigned long m_DeleteTag;    ///< ITK observer tag for the DeleteEvent.

    bool m_SelfCall;  ///< Guard flag to prevent re-entrant notification.
  };

  /**
   * \brief Read-only observer for a mitk::BaseProperty.
   *
   * Registers itself as an ITK observer on a const BaseProperty and receives
   * callbacks when the property changes or is deleted.
   *
   * \sa PropertyEditor
   * \sa PropertyObserver
   */
  class MITKCORE_EXPORT PropertyView : public PropertyObserver
  {
  public:
    /** \brief Constructor. Begins observing the given property.
     *
     * \param property the property to observe. Must not be nullptr.
     * \throw std::invalid_argument if \p property is nullptr.
     */
    PropertyView(const mitk::BaseProperty *property);

    /** \brief Destructor. Removes the ITK observer from the property. */
    ~PropertyView() override;

    /** \brief Callback for itk::ModifiedEvent on the observed property.
     *
     * \param e the ITK event object.
     */
    void OnModified(const itk::EventObject &e);

    /** \brief Callback for itk::DeleteEvent on the observed property.
     *
     * \param e the ITK event object.
     */
    void OnDelete(const itk::EventObject &e);

  protected:
    const mitk::BaseProperty *m_Property;  ///< The observed property (read-only).
  };

  /**
   * \brief Read-write observer for a mitk::BaseProperty.
   *
   * Similar to PropertyView, but holds a non-const pointer to the property,
   * allowing modification of the property value from within the observer.
   *
   * \sa PropertyView
   * \sa PropertyObserver
   */
  class MITKCORE_EXPORT PropertyEditor : public PropertyObserver
  {
  public:
    /** \brief Constructor. Begins observing the given property.
     *
     * \param property the property to observe and potentially modify. Must not be nullptr.
     * \throw std::invalid_argument if \p property is nullptr.
     */
    PropertyEditor(mitk::BaseProperty *property);

    /** \brief Destructor. Removes the ITK observer from the property. */
    ~PropertyEditor() override;

    /** \brief Callback for itk::ModifiedEvent on the observed property.
     *
     * \param e the ITK event object.
     */
    void OnModified(const itk::EventObject &e);

    /** \brief Callback for itk::DeleteEvent on the observed property.
     *
     * \param e the ITK event object.
     */
    void OnDelete(const itk::EventObject &e);

  protected:
    mitk::BaseProperty *m_Property;  ///< The observed property (read-write).
  };
}

#endif
