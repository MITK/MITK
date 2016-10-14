#pragma once

#include <berryMacros.h>

namespace berry {

struct EditorLayoutRec : public Object {

  berryObjectMacro(EditorLayoutRec);

  bool isCloseable;

  EditorLayoutRec() : isCloseable(true) {}
};

}
