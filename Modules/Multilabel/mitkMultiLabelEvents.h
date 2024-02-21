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
#define mitkMultiLabelEventMacroDeclaration(classname, super, IDType)         \
  /** \class classname */                                  \
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
  */
  class MITKMULTILABEL_EXPORT AnyLabelEvent : public itk::ModifiedEvent
  {
  public:
    using Self = AnyLabelEvent;
    using Superclass = itk::ModifiedEvent;
    const static mitk::Label::PixelType ANY_LABEL = std::numeric_limits<mitk::Label::PixelType>::max();

    AnyLabelEvent() = default;
    AnyLabelEvent(Label::PixelType labelValue);
    AnyLabelEvent(const Self & s);
    ~AnyLabelEvent() override;
    const char * GetEventName() const override;
    bool CheckEvent(const itk::EventObject * e) const override;
    itk::EventObject * MakeObject() const override;

    void SetLabelValue(Label::PixelType labelValue);
    Label::PixelType GetLabelValue() const;
  private:
    void operator=(const Self &);
    Label::PixelType m_LabelValue = std::numeric_limits<mitk::Label::PixelType>::max();
  };

  mitkMultiLabelEventMacroDeclaration(LabelAddedEvent, AnyLabelEvent, Label::PixelType);
  mitkMultiLabelEventMacroDeclaration(LabelModifiedEvent, AnyLabelEvent, Label::PixelType);
  mitkMultiLabelEventMacroDeclaration(LabelRemovedEvent, AnyLabelEvent, Label::PixelType);

  class MITKMULTILABEL_EXPORT LabelsChangedEvent : public itk::ModifiedEvent
  {
  public:
    using Self = LabelsChangedEvent;
    using Superclass = itk::ModifiedEvent;

    LabelsChangedEvent() = default;
    LabelsChangedEvent(std::vector<Label::PixelType> labelValues);
    LabelsChangedEvent(const Self& s);
    ~LabelsChangedEvent() override;
    const char* GetEventName() const override;
    bool CheckEvent(const itk::EventObject* e) const override;
    itk::EventObject* MakeObject() const override;

    void SetLabelValues(std::vector<Label::PixelType> labelValues);
    std::vector<Label::PixelType> GetLabelValues() const;
  private:
    void operator=(const Self&);
    std::vector<Label::PixelType> m_LabelValues;
  };

  class MITKMULTILABEL_EXPORT AnyGroupEvent : public itk::ModifiedEvent
  {
  public:
    using GroupIndexType = std::size_t;
    using Self = AnyGroupEvent;
    using Superclass = itk::ModifiedEvent;
    const static GroupIndexType ANY_GROUP = std::numeric_limits<GroupIndexType>::max();

    AnyGroupEvent() = default;
    AnyGroupEvent(GroupIndexType groupID);
    AnyGroupEvent(const Self& s);
    ~AnyGroupEvent() override;
    const char* GetEventName() const override;
    bool CheckEvent(const itk::EventObject* e) const override;
    itk::EventObject* MakeObject() const override;

    void SetGroupID(GroupIndexType groupID);
    GroupIndexType GetGroupID() const;
  private:
    void operator=(const Self&);
    GroupIndexType m_GroupID = std::numeric_limits<GroupIndexType>::max();
  };

  mitkMultiLabelEventMacroDeclaration(GroupAddedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);
  mitkMultiLabelEventMacroDeclaration(GroupModifiedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);
  mitkMultiLabelEventMacroDeclaration(GroupRemovedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);

}

#endif
