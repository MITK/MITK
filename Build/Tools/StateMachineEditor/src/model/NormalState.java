package model;

import java.util.List;

import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.swt.graphics.Image;
import org.jdom.Attribute;
import org.jdom.Element;

/**
 * @author Daniel
 */
public class NormalState extends States {
	/** A 16x16 pictogram of a normal state. */
	private static final Image RECTANGLE_ICON = createImage("rectangle16.gif");

	private static final long serialVersionUID = 1;

	private Element state = new Element("state");

	public Image getIcon() {
		return RECTANGLE_ICON;
	}

	public String toString() {
		return "Normalstate " + getStateName() + " ID " + getStateId();
	}

	public Element getStateElement() {
		return state;
	}

	public void setStateElement(Element state1) {
		state = state1;
		Point location = new Point(0,0);
		Dimension size = new Dimension(100, 50);
		List attributes = state1.getAttributes();
		for (int i = 0; i < attributes.size(); i++) {
			Attribute attr = (Attribute) attributes.get(i);
			if (attr.getName().equals("NAME")) {
				this.setStateName(state1.getAttributeValue("NAME"));
			}
			if (attr.getName().equals("ID")) {
				this.setStateId(state1.getAttributeValue("ID"));
			}
			if (attr.getName().equals("X_POS")) {
				location.x =  Integer.parseInt(state1.getAttributeValue("X_POS"));
			}
			if (attr.getName().equals("Y_POS")) {
				location.y = Integer.parseInt(state1.getAttributeValue("Y_POS"));
			}
			if (attr.getName().equals("HEIGHT")) {
				size.height = Integer.parseInt(state1.getAttributeValue("HEIGHT"));
			}
			if (attr.getName().equals("WIDTH")) {
				size.width = Integer.parseInt(state1.getAttributeValue("WIDTH"));
			}
		}
		this.setLocation(location);
		this.setSize(size);
	}
}

