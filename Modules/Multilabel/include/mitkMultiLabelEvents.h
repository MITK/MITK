/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelEvents_h
#define mitkMultiLabelEvents_h

#include <itkEventObject.h>
#include <mitkLabel.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
#define mitkMultiLabelEventMacroDeclaration(classname, super, IDType) \
  class MITKMULTILABEL_EXPORT classname : public super     \
  {                                                        \
  public:                                                  \
    using Self = classname;                                \
    using Superclass = super;                              \
    classname() = default;                                 \
    classname(IDType value);                               \
    classname(const Self & s);                             \
    virtual ~classname() override;                         \
    virtual const char *                                   \
    GetEventName() const override;                         \
    virtual bool                                           \
    CheckEvent(const itk::EventObject * e) const override; \
    virtual itk::EventObject *                             \
    MakeObject() const override;                           \
                                                           \
  private:                                                 \
    void                                                   \
    operator=(const Self &);                               \
  };                                                       \
  static_assert(true, "Compile time eliminated. Used to require a semi-colon at end of macro.")

#define mitkMultiLabelEventMacroDefinition(classname, super, IDType)         \
  classname::classname(const classname & s)                                  \
    : super(s){};                                                            \
  classname::classname(IDType value): super(value) {}                        \
  classname::~classname() {}                                                 \
  const char * classname::GetEventName() const { return #classname; }        \
  bool         classname::CheckEvent(const itk::EventObject * e) const       \
  {                                                                          \
    if (!super::CheckEvent(e)) return false;                                 \
    return (dynamic_cast<const classname *>(e) != nullptr);                  \
  }                                                                          \
  itk::EventObject * classname::MakeObject() const { return new classname; } \
  static_assert(true, "Compile time eliminated. Used to require a semi-colon at end of macro.")

  /**
   * \brief Base event class for all label-related events in a MultiLabelSegmentation.
   *
   * This event carries a label value identifying which label the event refers to.
   * Use the special ANY_LABEL value when adding an observer that should react to
   * events for any label, not just a specific one.
   *
   * \sa LabelAddedEvent, LabelModifiedEvent, LabelRemovedEvent, MultiLabelSegmentation
   */
  class MITKMULTILABEL_EXPORT AnyLabelEvent : public itk::ModifiedEvent
  {
  public:
    using Self = AnyLabelEvent;
    using Superclass = itk::ModifiedEvent;

    /** \brief Sentinel value indicating the event applies to any/all labels. */
    const static mitk::Label::PixelType ANY_LABEL = std::numeric_limits<mitk::Label::PixelType>::max();

    AnyLabelEvent() = default;

    /**
     * \brief Constructor with a specific label value.
     * \param[in] labelValue The label value this event refers to.
     */
    AnyLabelEvent(Label::PixelType labelValue);

    /** \brief Copy constructor. */
    AnyLabelEvent(const Self & s);

    ~AnyLabelEvent() override;

    /** \brief Returns the name of this event class. */
    const char * GetEventName() const override;

    /**
     * \brief Checks if a given event matches this event type and label value.
     * \param[in] e The event to check against.
     * \return true if the event matches.
     */
    bool CheckEvent(const itk::EventObject * e) const override;

    /** \brief Creates a copy of this event. */
    itk::EventObject * MakeObject() const override;

    /**
     * \brief Sets the label value this event refers to.
     * \param[in] labelValue The label value.
     */
    void SetLabelValue(Label::PixelType labelValue);

    /**
     * \brief Returns the label value this event refers to.
     * \return The label value, or ANY_LABEL if applicable to all labels.
     */
    Label::PixelType GetLabelValue() const;
  private:
    void operator=(const Self &);
    Label::PixelType m_LabelValue = std::numeric_limits<mitk::Label::PixelType>::max();
  };

  /** Event class that is used to indicated if a label is added in a MultiLabel class.
  *
  * It has a member that indicates the label id the event is referring to.
  * Use the ANY_LABEL value if you want to define an rvent (e.g. for adding an observer)
  * that reacts to every label and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(LabelAddedEvent, AnyLabelEvent, Label::PixelType);

  /** Event class that is used to indicated if a label is modified in a MultiLabel class.
  *
  * It has a member that indicates the label id the event is referring to.
  * Use the ANY_LABEL value if you want to define an rvent (e.g. for adding an observer)
  * that reacts to every label and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(LabelModifiedEvent, AnyLabelEvent, Label::PixelType);

  /** Event class that is used to indicated if a label is removed in a MultiLabel class.
  *
  * It has a member that indicates the label id the event is referring to.
  * Use the ANY_LABEL value if you want to define an rvent (e.g. for adding an observer)
  * that reacts to every label and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(LabelRemovedEvent, AnyLabelEvent, Label::PixelType);

  /**
   * \brief Event emitted once after a batch of label changes in a MultiLabelSegmentation.
   *
   * Unlike LabelAddedEvent, LabelModifiedEvent, and LabelRemovedEvent (which are emitted
   * per label), LabelsChangedEvent is sent only once after all modifications in an
   * operation are complete. For example, even if 4 labels are changed by a merge
   * operation, this event is sent only once with all affected label values.
   *
   * \sa AnyLabelEvent, LabelAddedEvent, LabelModifiedEvent, LabelRemovedEvent
   */
  class MITKMULTILABEL_EXPORT LabelsChangedEvent : public itk::ModifiedEvent
  {
  public:
    using Self = LabelsChangedEvent;
    using Superclass = itk::ModifiedEvent;

    LabelsChangedEvent() = default;

    /**
     * \brief Constructor with a vector of affected label values.
     * \param[in] labelValues The label values affected by the change.
     */
    LabelsChangedEvent(std::vector<Label::PixelType> labelValues);

    /** \brief Copy constructor. */
    LabelsChangedEvent(const Self& s);

    ~LabelsChangedEvent() override;

    /** \brief Returns the name of this event class. */
    const char* GetEventName() const override;

    /**
     * \brief Checks if a given event matches this event type.
     * \param[in] e The event to check against.
     * \return true if the event matches.
     */
    bool CheckEvent(const itk::EventObject* e) const override;

    /** \brief Creates a copy of this event. */
    itk::EventObject* MakeObject() const override;

    /**
     * \brief Sets the label values this event refers to.
     * \param[in] labelValues Vector of affected label values.
     */
    void SetLabelValues(std::vector<Label::PixelType> labelValues);

    /**
     * \brief Returns the label values this event refers to.
     * \return Vector of affected label values.
     */
    std::vector<Label::PixelType> GetLabelValues() const;
  private:
    void operator=(const Self&);
    std::vector<Label::PixelType> m_LabelValues;
  };

  /**
   * \brief Base event class for all group-related events in a MultiLabelSegmentation.
   *
   * This event carries a group index identifying which spatial group the event refers to.
   * Use the special ANY_GROUP value when adding an observer that should react to
   * events for any group, not just a specific one.
   *
   * \sa GroupAddedEvent, GroupModifiedEvent, GroupRemovedEvent, MultiLabelSegmentation
   */
  class MITKMULTILABEL_EXPORT AnyGroupEvent : public itk::ModifiedEvent
  {
  public:
    /** \brief Type for group indices. */
    using GroupIndexType = std::size_t;
    using Self = AnyGroupEvent;
    using Superclass = itk::ModifiedEvent;

    /** \brief Sentinel value indicating the event applies to any/all groups. */
    const static GroupIndexType ANY_GROUP = std::numeric_limits<GroupIndexType>::max();

    AnyGroupEvent() = default;

    /**
     * \brief Constructor with a specific group index.
     * \param[in] groupID The group index this event refers to.
     */
    AnyGroupEvent(GroupIndexType groupID);

    /** \brief Copy constructor. */
    AnyGroupEvent(const Self& s);

    ~AnyGroupEvent() override;

    /** \brief Returns the name of this event class. */
    const char* GetEventName() const override;

    /**
     * \brief Checks if a given event matches this event type and group index.
     * \param[in] e The event to check against.
     * \return true if the event matches.
     */
    bool CheckEvent(const itk::EventObject* e) const override;

    /** \brief Creates a copy of this event. */
    itk::EventObject* MakeObject() const override;

    /**
     * \brief Sets the group index this event refers to.
     * \param[in] groupID The group index.
     */
    void SetGroupID(GroupIndexType groupID);

    /**
     * \brief Returns the group index this event refers to.
     * \return The group index, or ANY_GROUP if applicable to all groups.
     */
    GroupIndexType GetGroupID() const;
  private:
    void operator=(const Self&);
    GroupIndexType m_GroupID = std::numeric_limits<GroupIndexType>::max();
  };

  /** Event class that is used to indicated if a group is added in a MultiLabel class.
  *
  * It has a member that indicates the group id the event is referring to.
  * Use the ANY_GROUP value if you want to define an event (e.g. for adding an observer)
  * that reacts to every group and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(GroupAddedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);

  /** Event class that is used to indicated if a group is modified in a MultiLabel class.
  *
  * It has a member that indicates the group id the event is referring to.
  * Use the ANY_GROUP value if you want to define an event (e.g. for adding an observer)
  * that reacts to every group and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(GroupModifiedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);

  /** Event class that is used to indicated if a group is removed in a MultiLabel class.
  *
  * It has a member that indicates the group id the event is referring to.
  * Use the ANY_GROUP value if you want to define an event (e.g. for adding an observer)
  * that reacts to every group and not just to a special one.
  */
  mitkMultiLabelEventMacroDeclaration(GroupRemovedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);

}

#endif
