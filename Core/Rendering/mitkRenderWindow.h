#ifndef MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD
#define MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD

#include <qgl.h> 

class iilItem; 

namespace mitk {

class RenderWindow : public QGLWidget 
{
public:
    //##ModelId=3E1EB4410304
    RenderWindow(QGLFormat glf, QWidget *parent = 0, const char *name = 0) : QGLWidget(glf, parent, name) {};

    //##ModelId=3E1EB4410318
    RenderWindow(QWidget *parent = 0, const char *name = 0) : QGLWidget(parent, name) {};

    //iil compatibility

    virtual RenderWindow* sharedWidget() const = 0;

    /*!
    \brief Gets the position of the top-left corner of the viewport.
    */
    virtual float x () const = 0;
    virtual float y () const = 0;

    /*!
    \brief Gets the size of the first image.
    */
    virtual unsigned int imageWidth () const = 0;
    virtual unsigned int imageHeight () const = 0;

    /*!
    \brief Checks if the item is in the viewport.
    */
    virtual bool itemInViewport (const iilItem* const item) = 0;
};

} // namespace mitk


#endif /* MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD */
