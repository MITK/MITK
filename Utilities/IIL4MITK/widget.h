#ifndef IIL_WIDGET_WRAPPER
#define IIL_WIDGET_WRAPPER

#include <qgl.h>
#include <assert.h>
#include <qptrlist.h> 

class iilItem;

class iilWidget : public QGLWidget 
{
public:
    iilWidget(QGLFormat glf, QWidget *parent = 0, const char *name = 0) : QGLWidget(glf, parent, name) {};
    iilWidget(QWidget *parent = 0, const char *name = 0) : QGLWidget(parent, name) {};
    ~iilWidget()
    {
    }
    //##Documentation
    //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
    virtual iilWidget* sharedWidget() const = 0;
    //##Documentation
    //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
    virtual isVisible (const iilItem* const item) = 0;
};

typedef iilWidget RenderWindowBase;

#endif
