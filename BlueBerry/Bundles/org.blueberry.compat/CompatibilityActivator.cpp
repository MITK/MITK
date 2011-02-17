/*=============================================================================

  Library: CTK

  Copyright (c) 2010 German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#include "CompatibilityActivator_p.h"
#include <QtPlugin>

CompatibilityActivator* CompatibilityActivator::instance = 0;

CompatibilityActivator::CompatibilityActivator()
  : context(0)
{
}

CompatibilityActivator::~CompatibilityActivator()
{
  
}

void CompatibilityActivator::start(ctkPluginContext* context)
{
  instance = this;
  this->context = context;
}

void CompatibilityActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

CompatibilityActivator* CompatibilityActivator::getInstance()
{
  return instance;
}

ctkPluginContext* CompatibilityActivator::getPluginContext() const
{
  return context;
}

Q_EXPORT_PLUGIN2(org_blueberry_compat, CompatibilityActivator)


