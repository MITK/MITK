#include "berryEditorLayout.h"

namespace berry
{

EditorLayout::EditorLayout(EditorLayoutRec::Pointer r) :
  rec(r)
{
  poco_assert(rec != 0);
}

bool EditorLayout::IsCloseable()
{
  return rec->isCloseable;
}

void EditorLayout::SetCloseable(bool closeable)
{
  rec->isCloseable = closeable;
}

}
