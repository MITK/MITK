MITK Rendering with QML prototype
=================================================

Summary
------------------

This mini-application "QuickRender" demonstrates how to build a Qt5 QML based
application that integrates MITK render windows.

The `QuickRender` application is located in `Examples/QuickRender.
The following plugins are showcased in a QtQuick environment:
 - DataManager
 - Level/Window
 - Transfer Function
 - Image Navigator

Interaction:
 - mouse clicks place the cross-hair
 - mouse scroll wheel moves to neighboring slice
 - holding the right mouse button and moving up/down changes the zoom level
 - holding the middle mouse button and mosing pans the scene (only when zoomed in)

 ![Screenshot](Screenshot.png=640x)

Code Changes
------------------

- added class `QmlMitkStdMultiItem`, which replaces the `QmlMitkFourRenderWindowWidget`
  - different approach to register QmlMitkRenderWindowItems
  - resembles more the QmitkStdMultiWidget (e.g. storing planes...)
  - added the respective qml file: MitkStdMultiItem.qml
- update class `QmlMitkRenderWindowItem`:
  - removed interaction legacy
  - updated interactions
- `QVTKQuickItem` now uses framebuffer objects to render external OpenGL context

Build Instructions
------------------

 - - install Qt 5.6 or 5.7.1
 - build MITK superbuild with
   - `MITK_BUILD_EXAMPLES` variable switched to `ON`
   - `MITK_USE_Qt5` switched to `ON`
   - `MITK_USE_CTK` switched to `ON`
   - `MITK_USE_BLUEBERRY` switched to `ON`

Open Issues
------------------

 - threading issue, since QtQuick's scene graph rendering will happen on a dedicated render thread
 - cleaner, fuller interfaces for the two new widgets
