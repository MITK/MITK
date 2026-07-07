/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAction_h
#define mitkAction_h

#include <mitkPropertyList.h>
#include <MitkCoreExports.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{
  /**
   * \brief Represents an action that is executed after a certain event in the state machine mechanism.
   *
   * An Action encapsulates an action ID and an optional set of properties that
   * parameterize the action's behavior. Actions are used by the interaction state
   * machine to execute responses to user input events.
   *
   * \ingroup Interaction
   * \sa PropertyList
   */
  class MITKCORE_EXPORT Action : public itk::Object
  {
  public:
    mitkClassMacroItkParent(Action, itk::Object);

    /**
     * \brief Static New method for SmartPointer-based construction.
     */
    mitkNewMacro1Param(Self, int);

    /**
     * \brief Add a property to the action's property list.
     *
     * \param[in] propertyKey The key under which to store the property.
     * \param[in] property The property instance to add.
     */
    void AddProperty(const char *propertyKey, BaseProperty *property);

    /**
     * \brief Get the ID of this action.
     * \return The action ID.
     */
    int GetActionId() const;

    /**
     * \brief Get a property by its key.
     *
     * \param[in] propertyKey The key of the property to retrieve.
     * \return Pointer to the property, or nullptr if not found.
     */
    mitk::BaseProperty *GetProperty(const char *propertyKey) const;

  protected:
    /**
     * \brief Construct an action with the given ID.
     * \param[in] actionId The unique action identifier.
     */
    Action(int actionId);

    /** \brief Destructor. */
    ~Action() override;

  private:
    int m_ActionId;                      ///< The unique ID of this action.
    PropertyList::Pointer m_PropertiesList; ///< Optional properties parameterizing the action.
  };

} // namespace mitk

#endif
