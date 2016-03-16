#pragma  once

#include <vtkGenericOpenGLRenderWindow.h>
#include <qopenglframebufferobject.h>
#include <QOpenGLFunctions>

class QVTKFramebufferObjectRenderer;

class vtkInternalOpenGLRenderWindow : public vtkGenericOpenGLRenderWindow
{
public:
	static vtkInternalOpenGLRenderWindow* New();
	vtkTypeMacro(vtkInternalOpenGLRenderWindow, vtkGenericOpenGLRenderWindow)

	virtual void OpenGLInitState();
	virtual void Render();
	void OpenGLEndState();
	void InternalRender();
	void SetFramebufferObject(QOpenGLFramebufferObject *fbo);

	QVTKFramebufferObjectRenderer *QtParentRenderer;

protected:
	vtkInternalOpenGLRenderWindow();
	~vtkInternalOpenGLRenderWindow();
	
};
