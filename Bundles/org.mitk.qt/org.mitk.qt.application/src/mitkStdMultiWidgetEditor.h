#ifndef MITKSTDMULTIWIDGETEDITOR_H_
#define MITKSTDMULTIWIDGETEDITOR_H_

#include <org.opencherry.ui/src/cherryEditorPart.h>

#include <QmitkStdMultiWidget.h>

class mitkStdMultiWidgetEditor : public cherry::EditorPart
{
public:
  cherryClassMacro(mitkStdMultiWidgetEditor);
  
  static const std::string EDITOR_ID;
  
  mitkStdMultiWidgetEditor();
  ~mitkStdMultiWidgetEditor();
  
  void Init(cherry::IEditorSite::Pointer site, cherry::IEditorInput::Pointer input);
  
  void* CreatePartControl(void* parent);
  void SetFocus();
  
  void DoSave() {}
  void DoSaveAs() {}
  bool IsDirty() { return false; }
  bool IsSaveAsAllowed() { return false; }
  
private:
  
  QmitkStdMultiWidget* m_StdMultiWidget;
};

#endif /*MITKSTDMULTIWIDGETEDITOR_H_*/
