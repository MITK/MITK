#pragma once

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

namespace berry {

struct BERRY_UI_QT IEditorLayout : public Object
{

  berryObjectMacro(berry::IEditorLayout)

  virtual  ~IEditorLayout() {};

  virtual bool IsCloseable() = 0;
  virtual void SetCloseable(bool closeable) = 0;
};

}
