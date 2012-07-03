BlueBerry/Bundles/org.blueberry.ui.qt/files.cmake[36m:[m  berryQt[1;31mEditorPart[m.cpp
BlueBerry/Bundles/org.blueberry.ui.qt/src/berryQtEditorPart.cpp[36m:[m#include "berryQt[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui.qt/src/berryQtEditorPart.cpp[36m:[mvoid Qt[1;31mEditorPart[m::CreatePartControl(void* parent)
BlueBerry/Bundles/org.blueberry.ui.qt/src/berryQtEditorPart.h[36m:[m#include <berry[1;31mEditorPart[m.h>
BlueBerry/Bundles/org.blueberry.ui.qt/src/berryQtEditorPart.h[36m:[mclass BERRY_UI_QT Qt[1;31mEditorPart[m : public [1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui.qt/src/berryQtEditorPart.h[36m:[m  berryObjectMacro(Qt[1;31mEditorPart[m);
BlueBerry/Bundles/org.blueberry.ui.qt/src/internal/berryQtWorkbenchPageTweaklet.cpp[36m:[mI[1;31mEditorPart[m::Pointer QtWorkbenchPageTweaklet::CreateError[1;31mEditorPart[m(const std::string&  /*partName*/, const std::string&  /*msg*/)
BlueBerry/Bundles/org.blueberry.ui.qt/src/internal/berryQtWorkbenchPageTweaklet.cpp[36m:[m  return I[1;31mEditorPart[m::Pointer(0);
BlueBerry/Bundles/org.blueberry.ui.qt/src/internal/berryQtWorkbenchPageTweaklet.h[36m:[m  I[1;31mEditorPart[m::Pointer CreateError[1;31mEditorPart[m(const std::string& partName, const std::string& msg);
BlueBerry/Bundles/org.blueberry.ui/files.cmake[36m:[m  src/berry[1;31mEditorPart[m.h
BlueBerry/Bundles/org.blueberry.ui/files.cmake[36m:[m  berry[1;31mEditorPart[m.cpp
BlueBerry/Bundles/org.blueberry.ui/manifest.cpp[36m:[mPOCO_BEGIN_NAMED_MANIFEST(berryI[1;31mEditorPart[m, berry::I[1;31mEditorPart[m)
BlueBerry/Bundles/org.blueberry.ui/schema/editors.exsd[36m:[m                  the name of a class that implements &lt;samp&gt;berry::I[1;31mEditorPart[m&lt;/samp&gt;. The attributes &lt;samp&gt;class&lt;/samp&gt;, &lt;samp&gt;command&lt;/samp&gt;, and &lt;samp&gt;launcher&lt;/samp&gt; are mutually exclusive. If this attribute is defined then &lt;samp&gt;contributorClass&lt;/samp&gt; should also be defined.
BlueBerry/Bundles/org.blueberry.ui/schema/editors.exsd[36m:[m                  <meta.attribute kind="java" basedOn="berry::[1;31mEditorPart[m"/>
BlueBerry/Bundles/org.blueberry.ui/schema/editors.exsd[36m:[mclass must implement &lt;samp&gt;org.eclipse.ui.I[1;31mEditorPart[m&lt;/samp&gt;.
BlueBerry/Bundles/org.blueberry.ui/schema/editors.exsd[36m:[m&lt;samp&gt;subclass org.eclipse.ui.[1;31mEditorPart[m&lt;/samp&gt; when 
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[m#include "berry[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[m[1;31mEditorPart[m::[1;31mEditorPart[m()
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mvoid [1;31mEditorPart[m::SetInput(IEditorInput::Pointer input)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mvoid [1;31mEditorPart[m::SetInputWithNotify(IEditorInput::Pointer input)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mvoid [1;31mEditorPart[m::SetContentDescription(const std::string& description)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mvoid [1;31mEditorPart[m::SetPartName(const std::string& partName)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mvoid [1;31mEditorPart[m::CheckSite(IWorkbenchPartSite::Pointer site)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mIEditorInput::Pointer [1;31mEditorPart[m::GetEditorInput() const
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mIEditorSite::Pointer [1;31mEditorPart[m::GetEditorSite() const
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mstd::string [1;31mEditorPart[m::GetTitleToolTip() const
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.cpp[36m:[mbool [1;31mEditorPart[m::IsSaveOnCloseNeeded() const
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m#include "berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m * <code>[1;31mEditorPart[m</code> subclass.
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m *   <li><code>I[1;31mEditorPart[m.init</code> - to initialize editor when assigned its site</li>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m *   <li><code>I[1;31mEditorPart[m.isDirty</code> - to decide whether a significant change has
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m *   <li><code>I[1;31mEditorPart[m.doSave</code> - to save contents of editor</li>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m *   <li><code>I[1;31mEditorPart[m.doSaveAs</code> - to save contents of editor</li>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m *   <li><code>I[1;31mEditorPart[m.isSaveAsAllowed</code> - to control Save As</li>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[mclass BERRY_UI [1;31mEditorPart[m : public WorkbenchPart , public I[1;31mEditorPart[m {
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m  Q_INTERFACES(berry::I[1;31mEditorPart[m)
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m  berryObjectMacro([1;31mEditorPart[m);
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m    [1;31mEditorPart[m();
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * for an editor.  For greater details, see <code>I[1;31mEditorPart[m</code>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * for an editor.  For greater details, see <code>I[1;31mEditorPart[m</code>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * Method declared on I[1;31mEditorPart[m.
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * Method declared on I[1;31mEditorPart[m.
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * Subclasses of <code>[1;31mEditorPart[m</code> must implement this method.  Within
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * for an editor.  For greater details, see <code>I[1;31mEditorPart[m</code>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * for an editor.  For greater details, see <code>I[1;31mEditorPart[m</code>
BlueBerry/Bundles/org.blueberry.ui/src/berryEditorPart.h[36m:[m     * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorInput.h[36m:[m * the model source for an <code>I[1;31mEditorPart[m</code>.
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorInput.h[36m:[m * An editor input is passed to an editor via the <code>I[1;31mEditorPart[m.init</code>
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorInput.h[36m:[m * @see org.blueberry.ui.I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[m * implementation is defined in <code>[1;31mEditorPart[m</code>.
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[m * @see org.blueberry.ui.part.[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[mstruct BERRY_UI I[1;31mEditorPart[m : public virtual IWorkbenchPart,
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[m  berryInterfaceMacro(I[1;31mEditorPart[m, berry);
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[m  virtual ~I[1;31mEditorPart[m()
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorPart.h[36m:[mQ_DECLARE_INTERFACE(berry::I[1;31mEditorPart[m, "org.blueberry.I[1;31mEditorPart[m")
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorReference.h[36m:[m#include "berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorReference.h[36m:[m * The I[1;31mEditorPart[m will not be instanciated until
BlueBerry/Bundles/org.blueberry.ui/src/berryIEditorReference.h[36m:[m    virtual I[1;31mEditorPart[m::Pointer GetEditor(bool restore) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIReusableEditor.h[36m:[m#include "berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/berryIReusableEditor.h[36m:[mstruct BERRY_UI IReusableEditor : public I[1;31mEditorPart[m {
BlueBerry/Bundles/org.blueberry.ui/src/berryIReusableEditor.h[36m:[m     * <p><b>Note:</b> Clients must fire the {@link I[1;31mEditorPart[m#PROP_INPUT }
BlueBerry/Bundles/org.blueberry.ui/src/berryISaveablePart.h[36m:[m * @see org.blueberry.ui.I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryISaveablesSource.h[36m:[m * behaviour implemented by {@link [1;31mEditorPart[m})</li>
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m#include "berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual bool CloseEditor(I[1;31mEditorPart[m::Pointer editor, bool save) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer GetActiveEditor() = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer FindEditor(IEditorInput::Pointer input) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual std::vector<I[1;31mEditorPart[m::Pointer> GetEditors() = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual std::vector<I[1;31mEditorPart[m::Pointer> GetDirtyEditors() = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input, const std::string& editorId) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input, const std::string& editorId,
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual bool SaveEditor(I[1;31mEditorPart[m::Pointer editor, bool confirm) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPage.h[36m:[m  virtual bool IsEditorPinned(I[1;31mEditorPart[m::Pointer editor) = 0;
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPart.h[36m:[m * <code>I[1;31mEditorPart[m</code>.
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPart.h[36m:[m * @see I[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryIWorkbenchPartConstants.h[36m:[m     * The property id for <code>I[1;31mEditorPart[m.getEditorInput()</code>.
BlueBerry/Bundles/org.blueberry.ui/src/berryWorkbenchPart.h[36m:[m * <code>[1;31mEditorPart[m</code>.
BlueBerry/Bundles/org.blueberry.ui/src/berryWorkbenchPart.h[36m:[m * @see org.blueberry.ui.part.[1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/berryWorkbenchPart.h[36m:[m   * Subclasses must invoke this method from <code>I[1;31mEditorPart[m.init</code>
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//I[1;31mEditorPart[m::Pointer HandlerUtil::GetActiveEditor(ExecutionEvent::Pointer event)
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//  return o.Cast<I[1;31mEditorPart[m>();
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//I[1;31mEditorPart[m::Pointer HandlerUtil::GetActiveEditorChecked(
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//  if (o.Cast<I[1;31mEditorPart[m>().IsNull())
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//        "I[1;31mEditorPart[m", o->GetClassName());
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.cpp[36m:[m//  return (I[1;31mEditorPart[m) o;
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.h[36m:[m  //static I[1;31mEditorPart[m::Pointer GetActiveEditor(ExecutionEvent::Pointer event);
BlueBerry/Bundles/org.blueberry.ui/src/handlers/berryHandlerUtil.h[36m:[m  //static I[1;31mEditorPart[m::Pointer GetActiveEditorChecked(ExecutionEvent::Pointer event);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorAreaHelper.cpp[36m:[mvoid EditorAreaHelper::CloseEditor(I[1;31mEditorPart[m::Pointer part)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorAreaHelper.cpp[36m:[mvoid EditorAreaHelper::MoveEditor(I[1;31mEditorPart[m::Pointer  /*part*/, int  /*position*/)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorAreaHelper.h[36m:[m    void CloseEditor(I[1;31mEditorPart[m::Pointer part);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorAreaHelper.h[36m:[m    void MoveEditor(I[1;31mEditorPart[m::Pointer part, int position);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[m#include "../berry[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorDescriptor::CreateEditor()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[m  I[1;31mEditorPart[m::Pointer extension(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[m      configurationElement->CreateExecutableExtension<I[1;31mEditorPart[m> (
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[m    extension = configurationElement->CreateExecutableExtension<I[1;31mEditorPart[m> (
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.cpp[36m:[m          WorkbenchRegistryConstants::ATT_CLASS, I[1;31mEditorPart[m::GetManifestName());
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.h[36m:[mstruct I[1;31mEditorPart[m;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorDescriptor.h[36m:[mpublic: SmartPointer<I[1;31mEditorPart[m> CreateEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mstd::vector<I[1;31mEditorPart[m::Pointer> EditorManager::CollectDirtyEditors()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> result;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    I[1;31mEditorPart[m::Pointer part = (*i)->GetPart(false).Cast<I[1;31mEditorPart[m>();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorManager::FindEditor(IEditorInput::Pointer input)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorManager::FindEditor(const std::string& editorId,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    return I[1;31mEditorPart[m::Pointer();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    I[1;31mEditorPart[m::Pointer part = editor->GetPart(false).Cast<I[1;31mEditorPart[m>();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mstd::vector<I[1;31mEditorPart[m::Pointer> EditorManager::GetDirtyEditors()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorManager::GetVisibleEditor()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    return I[1;31mEditorPart[m::Pointer(0);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m  return ref->GetPart(true).Cast<I[1;31mEditorPart[m>();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    I[1;31mEditorPart[m::Pointer part, EditorDescriptor::Pointer desc,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorManager::CreatePart(EditorDescriptor::Pointer desc) const
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m    I[1;31mEditorPart[m::Pointer result = desc->CreateEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m          I[1;31mEditorPart[m::Pointer editor = editorReference->GetEditor(false);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.cpp[36m:[m//      final I[1;31mEditorPart[m editor = ed.getEditor(false);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m#include "../berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> CollectDirtyEditors();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  I[1;31mEditorPart[m::Pointer FindEditor(IEditorInput::Pointer input);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  I[1;31mEditorPart[m::Pointer FindEditor(const std::string& editorId,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> GetDirtyEditors();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  I[1;31mEditorPart[m::Pointer GetVisibleEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m      I[1;31mEditorPart[m::Pointer part, EditorDescriptor::Pointer desc,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorManager.h[36m:[m  I[1;31mEditorPart[m::Pointer CreatePart(EditorDescriptor::Pointer desc) const;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m  //  I[1;31mEditorPart[m editor = getEditor(false);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorReference::GetEditor(bool restore)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m  return this->GetPart(restore).Cast<I[1;31mEditorPart[m> ();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m  I[1;31mEditorPart[m::Pointer part = this->GetEditor(false);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m    I[1;31mEditorPart[m::Pointer part =
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m        Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateError[1;31mEditorPart[m(label,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorReference::CreatePartHelper()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m  I[1;31mEditorPart[m::Pointer part;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorReference::GetEmptyEditor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m  I[1;31mEditorPart[m::Pointer part =
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m      Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateError[1;31mEditorPart[m("(Empty)", "");
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.cpp[36m:[m    return I[1;31mEditorPart[m::Pointer(0);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.h[36m:[m#include "../berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.h[36m:[m  I[1;31mEditorPart[m::Pointer GetEditor(bool restore);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.h[36m:[m  I[1;31mEditorPart[m::Pointer CreatePartHelper();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.h[36m:[m   * Creates and returns an empty editor (<code>Error[1;31mEditorPart[m</code>).
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorReference.h[36m:[m  I[1;31mEditorPart[m::Pointer GetEmptyEditor(SmartPointer<EditorDescriptor> descr);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.cpp[36m:[m    I[1;31mEditorPart[m::Pointer editor, WorkbenchPage* page,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.cpp[36m:[mI[1;31mEditorPart[m::Pointer EditorSite::Get[1;31mEditorPart[m()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.cpp[36m:[m  return this->GetPart().Cast<I[1;31mEditorPart[m>();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.h[36m:[m#include "../berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.h[36m:[m    EditorSite(IEditorReference::Pointer ref, I[1;31mEditorPart[m::Pointer editor,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryEditorSite.h[36m:[m    I[1;31mEditorPart[m::Pointer Get[1;31mEditorPart[m();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartList.cpp[36m:[mI[1;31mEditorPart[m::Pointer PartList::GetActiveEditor()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartList.cpp[36m:[m  return activeEditorReference.Expired() ? I[1;31mEditorPart[m::Pointer(0) : activeEditorReference.Lock()->GetEditor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartList.h[36m:[m  I[1;31mEditorPart[m::Pointer GetActiveEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartTester.cpp[36m:[mvoid PartTester::TestEditor(I[1;31mEditorPart[m::Pointer part)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartTester.h[36m:[m#include "../berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryPartTester.h[36m:[mpublic: static void TestEditor(I[1;31mEditorPart[m::Pointer part);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mSmartPointer<I[1;31mEditorPart[m> WorkbenchPage::ActivationList::GetTopEditor()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m    return I[1;31mEditorPart[m::Pointer(0);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer _topEditor = this->topEditor.Lock();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  bool isNewPartAnEditor = newPart.Cast<I[1;31mEditorPart[m> ().IsNotNull();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m      topEditor = newPart.Cast<I[1;31mEditorPart[m> ();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m    topEditor = newPart.Cast<I[1;31mEditorPart[m> ();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m    I[1;31mEditorPart[m::Pointer newEditor)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  if (part.Cast<I[1;31mEditorPart[m> () != 0)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer part = (ref == 0) ? I[1;31mEditorPart[m::Pointer(0)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mbool WorkbenchPage::CloseEditor(I[1;31mEditorPart[m::Pointer editor, bool save)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m        I[1;31mEditorPart[m::Pointer editorPart = reference->GetEditor(false);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::GetActiveEditor()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mstd::vector<I[1;31mEditorPart[m::Pointer> WorkbenchPage::GetEditors()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> result;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m    I[1;31mEditorPart[m::Pointer part = (*iter)->GetEditor(true);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mstd::vector<I[1;31mEditorPart[m::Pointer> WorkbenchPage::GetDirtyEditors()
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::FindEditor(IEditorInput::Pointer input)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::OpenEditor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::OpenEditor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::OpenEditorFromDescriptor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::BusyOpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer result;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::BusyOpenEditorFromDescriptor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer result;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::BusyOpenEditorBatched(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer editor;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m          I[1;31mEditorPart[m::Pointer editorToSave = editor;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m          return I[1;31mEditorPart[m::Pointer(0);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mI[1;31mEditorPart[m::Pointer WorkbenchPage::BusyOpenEditorFromDescriptorBatched(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer editor;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m  I[1;31mEditorPart[m::Pointer editor;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mvoid WorkbenchPage::ShowEditor(bool activate, I[1;31mEditorPart[m::Pointer editor)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mbool WorkbenchPage::IsEditorPinned(I[1;31mEditorPart[m::Pointer editor)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[mbool WorkbenchPage::SaveEditor(I[1;31mEditorPart[m::Pointer editor, bool confirm)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.cpp[36m:[m      if (newPart.Cast<I[1;31mEditorPart[m> () != 0)
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m    I[1;31mEditorPart[m::WeakPtr topEditor;
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m    void UpdateTopEditor(I[1;31mEditorPart[m::Pointer newEditor);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m    SmartPointer<I[1;31mEditorPart[m> GetTopEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  bool CloseEditor(I[1;31mEditorPart[m::Pointer editor, bool save);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer GetActiveEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> GetEditors();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  std::vector<I[1;31mEditorPart[m::Pointer> GetDirtyEditors();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer FindEditor(IEditorInput::Pointer input);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer OpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer OpenEditorFromDescriptor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer BusyOpenEditor(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer BusyOpenEditorFromDescriptor(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer BusyOpenEditorBatched(IEditorInput::Pointer input,
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  I[1;31mEditorPart[m::Pointer BusyOpenEditorFromDescriptorBatched(
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  void ShowEditor(bool activate, I[1;31mEditorPart[m::Pointer editor);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  bool IsEditorPinned(I[1;31mEditorPart[m::Pointer editor);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  bool SaveEditor(I[1;31mEditorPart[m::Pointer editor, bool confirm);
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPage.h[36m:[m  //        I[1;31mEditorPart[m getTopEditor() {
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPartReference.cpp[36m:[m#include "../berryI[1;31mEditorPart[m.h"
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPartReference.cpp[36m:[m  if (part.Cast<I[1;31mEditorPart[m> ().IsNull())
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchPartReference.cpp[36m:[m  return part.Cast<I[1;31mEditorPart[m> ()->IsDirty();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchWindow.cpp[36m:[m    I[1;31mEditorPart[m::Pointer editor = currentPage->GetActiveEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/berryWorkbenchWindow.cpp[36m:[m    I[1;31mEditorPart[m::Pointer editor = currentPage->GetActiveEditor();
BlueBerry/Bundles/org.blueberry.ui/src/internal/intro/berryEditorIntroAdapterPart.cpp[36m:[m  return introPart.IsNull() ? [1;31mEditorPart[m::GetPartName() : introPart->GetPartName();
BlueBerry/Bundles/org.blueberry.ui/src/internal/intro/berryEditorIntroAdapterPart.h[36m:[m#include <berry[1;31mEditorPart[m.h>
BlueBerry/Bundles/org.blueberry.ui/src/internal/intro/berryEditorIntroAdapterPart.h[36m:[mclass EditorIntroAdapterPart: public [1;31mEditorPart[m
BlueBerry/Bundles/org.blueberry.ui/src/tweaklets/berryWorkbenchPageTweaklet.h[36m:[m  virtual I[1;31mEditorPart[m::Pointer CreateError[1;31mEditorPart[m(const std::string& partName, const std::string& msg) = 0;
Build/Tools/StateMachineEditor/src/stateMachines/StateMachinesEditor.java[36m:[mimport org.eclipse.ui.I[1;31mEditorPart[m;
Build/Tools/StateMachineEditor/src/stateMachines/StateMachinesEditor.java[36m:[m    firePropertyChange(I[1;31mEditorPart[m.PROP_DIRTY);
Build/Tools/StateMachineEditor/src/stateMachines/StateMachinesEditor.java[36m:[m   * @see org.eclipse.ui.part.[1;31mEditorPart[m#setInput(org.eclipse.ui.IEditorInput)
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkCloseProjectAction.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkCloseProjectAction.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFileExitAction.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFileOpenAction.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFileOpenAction.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFileOpenAction.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFunctionality.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFunctionality.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor =
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkFunctionality.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkStdMultiWidgetEditor.h[36m:[m#include <berryQt[1;31mEditorPart[m.h>
CoreUI/Bundles/org.mitk.gui.qt.common/src/QmitkStdMultiWidgetEditor.h[36m:[m  public berry::Qt[1;31mEditorPart[m, virtual public berry::IPartListener
Modules/Bundles/org.mitk.gui.qt.datamanager/src/QmitkDataManagerView.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.datamanager/src/QmitkDataManagerView.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor;
Modules/Bundles/org.mitk.gui.qt.datamanager/src/QmitkDataManagerView.cpp[36m:[m  std::vector<berry::I[1;31mEditorPart[m::Pointer> editors =
Modules/Bundles/org.mitk.gui.qt.datamanager/src/QmitkDataManagerView.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
Modules/Bundles/org.mitk.gui.qt.diffusionimaging/src/internal/QmitkPartialVolumeAnalysisView.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.diffusionimagingapp/src/internal/QmitkDiffusionImagingAppIntroPart.cpp[36m:[m      berry::I[1;31mEditorPart[m::Pointer editor = GetIntroSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
Modules/Bundles/org.mitk.gui.qt.dtiatlasapp/src/internal/QmitkDTIAtlasAppIntroPart.cpp[36m:[m      berry::I[1;31mEditorPart[m::Pointer editor = GetIntroSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
Modules/Bundles/org.mitk.gui.qt.examples/src/internal/simplemeasurement/QmitkSimpleMeasurement.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtFileOpenAction.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtFileOpenAction.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtFileOpenAction.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtFileSaveProjectAction.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtFileSaveProjectAction.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtWorkbenchWindowAdvisor.cpp[36m:[m berry::I[1;31mEditorPart[m::Pointer activeEditor;
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtWorkbenchWindowAdvisor.cpp[36m:[m berry::I[1;31mEditorPart[m::Pointer activeEditor;
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtWorkbenchWindowAdvisor.h[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.ext/src/QmitkExtWorkbenchWindowAdvisor.h[36m:[m  berry::I[1;31mEditorPart[m::WeakPtr lastActiveEditor;
Modules/Bundles/org.mitk.gui.qt.imagenavigator/src/internal/QmitkImageNavigatorView.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor =
Modules/Bundles/org.mitk.gui.qt.imagenavigator/src/internal/QmitkImageNavigatorView.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
Modules/Bundles/org.mitk.gui.qt.imagestatistics/src/internal/QmitkImageStatisticsView.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.materialeditor/src/internal/QmitkMITKSurfaceMaterialEditorView.cpp[36m:[m#include "berryI[1;31mEditorPart[m.h"
Modules/Bundles/org.mitk.gui.qt.measurement/src/internal/QmitkMeasurement.cpp[36m:[m#include <berryI[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonEditor.h[36m:[m#include <berryQt[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonEditor.h[36m:[mclass QmitkPythonEditor : public berry::Qt[1;31mEditorPart[m, virtual public berry::IPartListener
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonScriptEditor.h[36m:[m#include <berryQt[1;31mEditorPart[m.h>
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonScriptEditor.h[36m:[mclass QmitkPythonScriptEditor : public QWidget, public QmitkPythonPasteClient //, berry::Qt[1;31mEditorPart[m
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonVariableStack.cpp[36m:[m  berry::I[1;31mEditorPart[m::Pointer editor =
Modules/Bundles/org.mitk.gui.qt.python.console/src/internal/QmitkPythonVariableStack.cpp[36m:[m    berry::I[1;31mEditorPart[m::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
