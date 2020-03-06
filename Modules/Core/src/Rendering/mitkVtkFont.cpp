#include <mitkVtkFont.h>
#include <PathUtilities.h>

static std::string s_FontPath;

void mitk::setUnicodeFont(vtkTextProperty* prop)
{
  prop->SetFontFamily(VTK_FONT_FILE);
  if (s_FontPath.empty())
  {
    s_FontPath = Utilities::preferredPath(Utilities::absPath(std::string("Fonts/DejaVuSans.ttf")));
  }
  prop->SetFontFile(s_FontPath.c_str());
}
