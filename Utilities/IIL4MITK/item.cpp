#include "widget.h"
#include "item.h"

iilItem::iilItem (QObject* parent, const char* name)
        : QObject (parent, name), _x (0.0), _y (0.0), _red (1.0), _green (1.0), _blue (1.0), _alpha (1.0), _lineWidth (0.0), _constraint (0), _visible (true), _pickable (false), _scalable (true)
{
}

void
iilItem::remove (iilWidget* )
{
}

void
iilItem::move (const float x, const float y)
{
    _x = x;
    _y = y;
}

float
iilItem::x () const
{
    return _x;
}

float
iilItem::y () const
{
    return _y;
}

float
iilItem::width () const
{
    return 0.0;
}

float
iilItem::height () const
{
    return 0.0;
}


void
iilItem::setColor (const float red, const float green, const float blue, const float alpha)
{
    _red = red;
    _green = green;
    _blue = blue;
    _alpha = alpha;
}

float
iilItem::red () const
{
    return _red;
}

float
iilItem::green () const
{
    return _green;
}

float
iilItem::blue () const
{
    return _blue;
}

float
iilItem::alpha () const
{
    return _alpha;
}

void 
iilItem::setLineWidth (const float width)
{
	_lineWidth = width;
}

float 
iilItem::lineWidth () const
{
	return _lineWidth;
}

void
iilItem::setPickable (const bool on)
{
    _pickable = on;
}

bool
iilItem::pickable () const
{
    return _pickable;
}

iilItem*
iilItem::picked (iilWidget*, const float x, const float y)
{
    if (!pickable () || !visible ()) return NULL;

    bool p = !((x < this->x ()) || (y < this->y ()) || (x > this->x () + this->width ()) || (y > this->y () + this->height ()));
    iilItem* obj = (p ? this : NULL);
    return obj;
}

void 
iilItem::setScalable (const bool scalable)
{
	_scalable = scalable;
}

bool 
iilItem::scalable () const
{
	return _scalable;
}

void
iilItem::setConstraint (iilWidget* widget) {
	_constraint = widget;
}

iilWidget* 
iilItem::constraint () const
{
	return _constraint;
}

void
iilItem::setVisible (const bool visible)
{
    _visible = visible;
}

bool
iilItem::visible () const
{
    return _visible;
}

void
iilItem::hide () 
{
	setVisible (false);
}

void
iilItem::show ()
{
	setVisible (true);
}
