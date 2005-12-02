package model;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import org.jdom.Comment;
import org.jdom.Element;

public class Action {

	private static final long serialVersionUID = 1;
	
	private Element action = null;

	/**
	 * @return the jDOM action element
	 */
	public Element getActionElement() {
		return action;
	}
	
	/**
	 * sets the jDOM element of this action
	 * @param action1 the jDOM element
	 */
	public void setActionElement(Element action1) {
		action = action1;
	}
	
	/**
	 * @return the id of this action
	 */
	public String getActionId() {
		String action2 = action.getAttributeValue("ID");
		return action2;
	}
	
	/**
	 * sets the id on this action
	 * @param id the new id
	 */
	public void setAction(String id) {
		this.getActionElement().setAttribute("ID", id);
	}
	
	/**
	 * @return all parameters the action element has as children
	 */
	public List getParameter() {
		List paraList = action.getChildren();
		List allVectors = new ArrayList();
		for (int i = 0; i < paraList.size(); i++) {
			Object paraEle1 = paraList.get(i);
			if(!(paraEle1 instanceof Comment)) {
				Element paraEle = (Element) paraEle1;
				Vector para = new Vector(4);
				para.add(0, paraEle.getAttributeValue("NAME"));
				para.add(1, paraEle.getName());
				para.add(2, paraEle.getAttributeValue("VALUE"));
				List comment = paraEle.getContent();
				para.add(3, "");
				for (int j = 0; j < comment.size(); j++) {
					Object o = comment.get(j);
					if (o instanceof Comment) {
						Comment ele = (Comment) o;
						para.add(3,ele.getText());
					}
				}
				allVectors.add(para);
			}
		}
		return allVectors;
	}
	
	/**
	 * @return the comment on this action
	 */
	public String getActionComment() {
		List actionComment = action.getContent();
		for (int i = 0; i < actionComment.size(); i++) {
			Object o = actionComment.get(i);
			if (o instanceof Comment) {
				Comment ele = (Comment) o;
				return ele.getText();
			}
		}
		return "";
	}
}
