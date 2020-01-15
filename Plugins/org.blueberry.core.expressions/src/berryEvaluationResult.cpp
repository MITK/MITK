/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEvaluationResult.h"

namespace berry {

const int EvaluationResult::FALSE_VALUE = 0;
const int EvaluationResult::TRUE_VALUE = 1;
const int EvaluationResult::NOT_LOADED_VALUE = 2;

const SmartPointer<const EvaluationResult> EvaluationResult::FALSE_EVAL(new EvaluationResult(EvaluationResult::FALSE_VALUE));
const SmartPointer<const EvaluationResult> EvaluationResult::TRUE_EVAL(new EvaluationResult(EvaluationResult::TRUE_VALUE));
const SmartPointer<const EvaluationResult> EvaluationResult::NOT_LOADED(new EvaluationResult(NOT_LOADED_VALUE));

bool EvaluationResult::operator==(const Object* result) const
{
  if(const EvaluationResult* o = dynamic_cast<const EvaluationResult*>(result))
  {
    return this->fValue == o->fValue;
  }
  return false;
}

bool EvaluationResult::operator!=(const Object* result) const
{
  return !(this == result);
}

const SmartPointer<const EvaluationResult> EvaluationResult::AND[3][3] = {
    //                    FALSE                      TRUE                  NOT_LOADED
    /* FALSE   */ { EvaluationResult::FALSE_EVAL, EvaluationResult::FALSE_EVAL, EvaluationResult::FALSE_EVAL   },
    /* TRUE    */ { EvaluationResult::FALSE_EVAL, EvaluationResult::TRUE_EVAL,  EvaluationResult::NOT_LOADED  },
    /* PNL     */ { EvaluationResult::FALSE_EVAL, EvaluationResult::NOT_LOADED, EvaluationResult::NOT_LOADED  }
};

const SmartPointer<const EvaluationResult> EvaluationResult::OR[3][3] = {
    //                      FALSE                   TRUE                  NOT_LOADED
    /* FALSE   */ { EvaluationResult::FALSE_EVAL, EvaluationResult::TRUE_EVAL, EvaluationResult::NOT_LOADED  },
    /* TRUE    */ { EvaluationResult::TRUE_EVAL,  EvaluationResult::TRUE_EVAL, EvaluationResult::TRUE_EVAL    },
    /* PNL     */ { EvaluationResult::NOT_LOADED, EvaluationResult::TRUE_EVAL,   EvaluationResult::NOT_LOADED  }
};

const SmartPointer<const EvaluationResult> EvaluationResult::NOT[3] = {
    //    FALSE                        TRUE                  NOT_LOADED
    EvaluationResult::TRUE_EVAL, EvaluationResult::FALSE_EVAL, EvaluationResult::NOT_LOADED
};


EvaluationResult::EvaluationResult(int value)
{
  fValue= value;
}

EvaluationResult::EvaluationResult(const EvaluationResult &o)
  : Object()
  , fValue(o.fValue)
{
}

EvaluationResult::ConstPointer
EvaluationResult::And(const EvaluationResult::ConstPointer& other) const
{
  return AND[fValue][other->fValue];
}

EvaluationResult::ConstPointer
EvaluationResult::Or(const EvaluationResult::ConstPointer& other) const
{
  return OR[fValue][other->fValue];
}


EvaluationResult::ConstPointer
EvaluationResult::Not() const
{
  return NOT[fValue];
}

EvaluationResult::ConstPointer
EvaluationResult::ValueOf(bool b)
{
  return (b ? TRUE_EVAL : FALSE_EVAL);
}


QString EvaluationResult::ToString() const
{
  switch (fValue) {
  case 0:
    return "false";
  case 1:
    return "true";
  case 2:
    return "not_loaded";
  default:
    Q_ASSERT(false);
    return "";
  }
}

}  // namespace berry
