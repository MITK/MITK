#ifndef _ITEM_H_
#define _ITEM_H_

#include <GL/gl.h>

class iilWidget;

/*!
\brief The items are objects that can be displayed by 
the widget. When the widget demands an update of the display, 
each item draws itself into the viewport. If the user wants to
interact with some item, each item provides the information
whether it has been picked by the user.

\todo Add a scale method.
\todo Add a parameter iilWidget to the picking method. Done.
*/

class iilItem {

public:

    /*!
    \brief The constructor.
    @param parent the parent item
    @param name the object name
    */
    iilItem();

    /*!
    \brief Displays the item.
    \note Make sure that the proper OpenGL context has been 
    made current.
    \todo Should the method return whether something has been
    rendered?  
    */
    virtual void display (iilWidget* widget) = 0;

    /*!
    \brief Frees the resources of the item for the given widget.
    \note Make sure that the proper OpenGL context has been 
    made current.
    */
    virtual void remove (iilWidget* widget);

    /*!
    \brief Sets the position of the item.
    */
    void move (const float x, const float y);

    /*!
    \brief Gets the position of the item.
    */
    float x () const;
    float y () const;

    /*!
    \brief Gets the size of the item.
    \note The method may be helpful for picking.
    */
    virtual float width () const;
    virtual float height () const;

    /*!
    \brief Sets the color of the item.
    @param red,green,blue the channels of the rgb color model
    @param alpha the opacity. If alpha is equal to one, the
    item is rendered opaque, otherwise it is rendered 
    semi-transparent. If alpha is zero, the item won't be visible.
    */
    virtual void setColor (const float red, const float green, const float blue, const float alpha);

    /*!
    \brief Gets the color channels.
    */
    float red () const;
    float green () const;
    float blue () const;
    float alpha () const;

	/*!
	\brief Sets the widht of the isocontour.
	@param width the line width
	*/
	void setLineWidth (const float width);

	/*!
	\brief Gets the width of the isocontour.
	*/
	float lineWidth () const;

    /*!
    \brief Sets the flag whether the user can pick the item.
    */
    void setPickable (const bool on);

    /*!
    \brief Checks if the user can pick the item.
    @returns If the item is allowed to be picked, true is returned.
    */
    bool pickable () const;

    /*!
    \brief Checks if the item has been picked by the user.
    @param widget the widget which caused an event
    @param x,y the point the user has picked
    @returns the picked item, or null if no item has been picked 
    */
    virtual iilItem* picked (iilWidget* widget, const float x, const float y);

    /*!
    \brief Sets if items should be scaled when the viewport is zoomed. 
    If so, items will grow as far as the viewport is zoomed. Otherwise, 
    items will keep its size constant.
    */
    void setScalable (const bool scalable);

    /*!
    \brief Checks if items will be scaled when the viewport changes.
    */
    bool scalable () const;

    /*!
    \brief Sets the given widget as a constraint for rendering, i.e.,
    the contents are rendered only if the widget that demands the 
    rendering is the same as the given one. 
    @param widget the widget which the rendering is constraint to. If
    null is given, the constraint will be removed. 
    */
    void setConstraint (iilWidget* widget);

    /*!
    \brief Gets the widget which the rendering is constraint to.
    */
    iilWidget* constraint () const;

    /*!
    \brief Makes the item visible or hidden, respectively.
    @param visible If true the item is displayed, otherwise it is not.
    */
    void setVisible (const bool visible);

    /*!
    \brief Checks if the item will be visible.
    @returns If the item will be displayed, true is returned.
    */
    bool visible () const;

    /*!
      \brief Shows the item.
    */
    void show ();

    /*!
      \brief Hides the item.
    */
    void hide ();

private:

    /*!
    \brief The position of the item.
    */
    float _x, _y;

    /*!
    \brief The color channels.
    */
    float _red, _green, _blue, _alpha;

	/*!
	\brief The line width.
	*/
	float _lineWidth;

    /*!
    \brief The widget which the rendering is constraint to.
    */
    iilWidget* _constraint;

    /*!
    \brief Flags if the item will be displayed during 
    the next rendering.
    */
    bool _visible;

    /*!
    \brief Flags if the item is allowed to be picked. 
    */
    bool _pickable;

    /*!
    \brief Flags if the item is scalable. If so, 
    items will grow as far as the viewport is zoomed. 
    Otherwise, items will keep its size constant.
    */
    bool _scalable;

};

#endif
