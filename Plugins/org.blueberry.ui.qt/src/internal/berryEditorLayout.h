#pragma once

#include "berryPageLayout.h"
#include "berryEditorLayoutRec.h"

namespace berry
{

class EditorLayout : public IEditorLayout
{

private:

  EditorLayoutRec::Pointer rec;

public:

  EditorLayout(EditorLayoutRec::Pointer rec);

  bool IsCloseable() override;
  void SetCloseable(bool closeable) override;
};

}
