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

#include "berryCommandCategory.h"
#include "berryCommandCategoryEvent.h"

#include <Poco/Exception.h>
#include <sstream>

namespace berry {

 void CommandCategory::AddCategoryListener(
      ICommandCategoryListener::Pointer categoryListener) {
    if (!categoryListener) {
      throw Poco::NullPointerException("Category listener cannot be null");
    }

    categoryEvents.AddListener(categoryListener);
  }

  void CommandCategory::Define(const std::string& name, const std::string& description) {
    if (name.empty()) {
      throw Poco::InvalidArgumentException(
          "The name of a command cannot be empty"); //$NON-NLS-1$
    }

    const bool definedChanged = !this->defined;
    this->defined = true;

    const bool nameChanged = !(this->name == name);
    this->name = name;

    const bool descriptionChanged = !(this->description == description);
    this->description = description;

    const CommandCategoryEvent::Pointer categoryEvent(new CommandCategoryEvent(CommandCategory::Pointer(this), definedChanged,
        descriptionChanged, nameChanged));
    this->FireCategoryChanged(categoryEvent);
  }

  void CommandCategory::RemoveCategoryListener(
      ICommandCategoryListener::Pointer categoryListener) {
    if (!categoryListener) {
      throw Poco::NullPointerException("CommandCategory listener cannot be null");
    }

    categoryEvents.RemoveListener(categoryListener);
  }

  std::string CommandCategory::ToString() {
    if (str.empty()) {
      std::stringstream stringBuffer;
      stringBuffer << "Category(" << id << "," << name << "," << description << "," << defined << ")";
      str = stringBuffer.str();
    }
    return str;
  }

  void CommandCategory::Undefine() {
    str = "";

    const bool definedChanged = defined;
    defined = false;

    const bool nameChanged = !name.empty();
    name = "";

    const bool descriptionChanged = !description.empty();
    description = "";

    const CommandCategoryEvent::Pointer categoryEvent(new CommandCategoryEvent(CommandCategory::Pointer(this), definedChanged,
        descriptionChanged, nameChanged));
    this->FireCategoryChanged(categoryEvent);
  }

  CommandCategory::CommandCategory(const std::string& id) :
  NamedHandleObject(id)
  {

  }

  void CommandCategory::FireCategoryChanged(const CommandCategoryEvent::Pointer categoryEvent) {
    if (!categoryEvent) {
      throw Poco::NullPointerException("Command category event cannot be null");
    }
    categoryEvents.categoryChanged(categoryEvent);
  }

}
