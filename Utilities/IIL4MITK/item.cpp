#include "widget.h"
#include "item.h"

iil4mitkItem::iil4mitkItem ()
        : _x (0.0), _y (0.0), _red (1.0), _green (1.0), _blue (1.0), _alpha (1.0), _lineWidth (0.0), _constraint (0), _visible (true), _pickable (false), _scalable (true)
{
}

void
iil4mitkItem::remove (iil4mitkWidget* )
{
}

void
iil4mitkItem::move (const float x, const float y)
{
    _x = x;
    _y = y;
}

float
iil4mitkItem::x () const
{
    return _x;
}

float
iil4mitkItem::y () const
{
    return _y;
}

float
iil4mitkItem::width () const
{
    return 0.0;
}

float
iil4mitkItem::height () const
{
    return 0.0;
}


void
iil4mitkItem::setColor (const float red, const float green, const float blue, const float alpha)
{
    _red = red;
    _green = green;
    _blue = blue;
    _alpha = alpha;
}

float
iil4mitkItem::red () const
{
    return _red;
}

float
iil4mitkItem::green () const
{
    return _green;
}

float
iil4mitkItem::blue () const
{
    return _blue;
}

float
iil4mitkItem::alpha () const
{
    return _alpha;
}

void 
iil4mitkItem::setLineWidth (const float width)
{
	_lineWidth = width;
}

float 
iil4mitkItem::lineWidth () const
{
	return _lineWidth;
}

void
iil4mitkItem::setPickable (const bool on)
{
    _pickable = on;
}

bool
iil4mitkItem::pickable () const
{
    return _pickable;
}

iil4mitkItem*
iil4mitkItem::picked (iil4mitkWidget*, const float x, const float y)
{
    if (!pickable () || !visible ()) return NULL;

    bool p = !((x < this->x ()) || (y < this->y ()) || (x > this->x () + this->width ()) || (y > this->y () + this->height ()));
    iil4mitkItem* obj = (p ? this : NULL);
    return obj;
}

void 
iil4mitkItem::setScalable (const bool scalable)
{
	_scalable = scalable;
}

bool 
iil4mitkItem::scalable () const
{
	return _scalable;
}

void
iil4mitkItem::setConstraint (iil4mitkWidget* widget) {
	_constraint = widget;
}

iil4mitkWidget* 
iil4mitkItem::constraint () const
{
	return _constraint;
}

void
iil4mitkItem::setVisible (const bool visible)
{
    _visible = visible;
}

bool
iil4mitkItem::visible () const
{
    return _visible;
}

void
iil4mitkItem::hide () 
{
	setVisible (false);
}

void
iil4mitkItem::show ()
{
	setVisible (true);
}
