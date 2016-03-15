MITK Rendering with QML prototype
=================================================

Summary
------------------

This mini-application "QuickRender" demonstrates how to possibly build a Qt5 QML based
application that integrates MITK render windows.

The `QuickRender` application is located in `Examples/QuickRender`
 - it provides no "File/Open" dialog
 - should be called with a file loadable by MITK, e.g. a 3D ![NRRD file](http://mitk.org/git/?p=MITK-Data.git;a=blob_plain;f=Pic3D.nrrd;hb=HEAD)

`./QuickRender ~/NRRD-testimages/nrrd3dblock.nrrd`

![Screenshot](http://maleike.github.io/MITK/quickrender.png)

Interaction:
 - mouse clicks place the cross-hair
 - mouse scroll wheel moves to neighboring slice
 - holding the right mouse button and moving up/down changes the zoom level
 - holding the middle mouse button and mosing pans the scene (only when zoomed in)

Code Changes
------------------

A new module QmlMitk provides
 - class `QmlMitkRenderWindowItem`: provides functionality like `QmitkRenderWindowm` based on `QVTKQuickItem` (from ![OpenView](https://github.com/Kitware/openview))
 - class `QmlMitkFourRenderWindowWidget`: simplified replacement for `QmitkStdMultiWidget`, a minimal combination of four `QmlMitkFourRenderWindowWidget`s
 - a Qt 5, QtQuick dependency

Build Instructions
------------------

 - build Qt 5.1
 - place the Qt 5.1 qmake application in your environment `PATH`
 - build MITK superbuild with
   - `MITK_BUILD_EXAMPLES` variable switched to `ON`
   - advanced option `MITK_USE_CTK` switched to `OFF`
   - `MITK_USE_BLUEBERRY` and `MITK_USE_QT` switched to `OFF` (MITK_USE_QT finds Qt 5)

Open Issues
------------------

 - Build system integration of Qt 5 dependency
 - Build system integration of OpenView package
 - cleaner, fuller interfaces for the two new widgets
 - real solution for QmlMitkBigRenderLock
