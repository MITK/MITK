package dom;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.TransformerException;

import org.jdom.Comment;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

public class ReadActionAndEventDOMTree {
	
	private Document doc = null;
	private Map eventIdMap = new HashMap();
	private Map eventNameMap = new HashMap();
	private Map eventCategoryMap = new HashMap();
	private Map actionIdMap = new HashMap();
	private Map actionNameMap = new HashMap();
	private Map actionCategoryMap = new HashMap();
	private List allEventCats = new ArrayList();
	private List allActionCats = new ArrayList();
	private File filename = null;

	/**
	 * constructor for ReadActionAndEventDOMTree
	 * builds the DOMTree from the xml file and Maps the id´s names and categories
	 * @param filename
	 */
	public ReadActionAndEventDOMTree(File filename) {
		try {
			// Build the document with SAX and Xerces, no validation
        	SAXBuilder builder = new SAXBuilder();
        	// Create the document
        	this.filename = filename;
        	if (!filename.exists()) {
        		Element root = new Element("mitkInteraktionEvents");
       		 	Element events = new Element("events");
       		 	Element evcat = new Element ("eventCategory");
       		 	evcat.setAttribute("NAME", "NullEvent");
       		 	Element event = new Element("event");
       		 	event.setAttribute("NAME", "EIDNULLEVENT");
       		 	event.setAttribute("ID", "0");
       		 	evcat.addContent(event);
       		 	events.addContent(evcat);
       		 	root.addContent(events);
       		 	Element actions = new Element("actions");
    		 	Element accat = new Element ("actionCategory");
    		 	accat.setAttribute("NAME", "DoNothing");
    		 	Element action = new Element("action");
    		 	action.setAttribute("NAME", "AcDONOTHING");
    		 	action.setAttribute("ID", "0");
    		 	accat.addContent(action);
    		 	actions.addContent(accat);
    		 	root.addContent(actions);
       		 	doc = new Document(root);
        	}
        	else {
        		doc = builder.build(filename);
        		List allEventCategories = getAllEventCategories();
        		for (int i = 0; i < allEventCategories.size(); i++) {
        			Element cat = (Element) allEventCategories.get(i);
        			if (!allEventCats.contains(cat.getAttributeValue("NAME"))) {
        				allEventCats.add(cat.getAttributeValue("NAME"));
        			}
        			List allEvents = getEvents(cat.getAttributeValue("NAME"));
        			for (int j = 0; j < allEvents.size(); j++) {
        				Element ev = (Element) allEvents.get(j);
        				eventCategoryMap.put(ev.getAttributeValue("NAME"), cat.getAttributeValue("NAME"));
        				eventIdMap.put(ev.getAttributeValue("ID"), ev.getAttributeValue("NAME"));
        				eventNameMap.put(ev.getAttributeValue("NAME"), ev.getAttributeValue("ID"));
        			}
        		}
        		List allActionCategories = getAllActionCategories();
        		for (int i = 0; i < allActionCategories.size(); i++) {
        			Element cat = (Element) allActionCategories.get(i);
        			if (!allActionCats.contains(cat.getAttributeValue("NAME"))) {
        				allActionCats.add(cat.getAttributeValue("NAME"));
        			}
        			List allActions = getActions(cat.getAttributeValue("NAME"));
        			for (int j = 0; j < allActions.size(); j++) {
        				Element act = (Element) allActions.get(j);
        				actionCategoryMap.put(act.getAttributeValue("NAME"), cat.getAttributeValue("NAME"));
        				actionIdMap.put(act.getAttributeValue("ID"), act.getAttributeValue("NAME"));
        				actionNameMap.put(act.getAttributeValue("NAME"), act.getAttributeValue("ID"));
        			}
        		}
        	}
		} catch (Exception e) {
    		e.printStackTrace();
    	}
	}
	
	/**
	 * saves the DOMTree to the same xml file and 
	 * calls the method to convert the xml file to a C++ header
	 */
	public void writeTree() {
		Format format = Format.getPrettyFormat();
		format.setEncoding("utf-8");
		XMLOutputter xmlOut = new XMLOutputter(format);
		try {
			xmlOut.output(doc, new FileOutputStream(filename));
			new ConvertToHeader(filename);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (TransformerException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * @return all event category elements
	 */
	public List getAllEventCategories() {
		Element events = doc.getRootElement().getChild("events", doc.getRootElement().getNamespace());
		return events.getChildren("eventCategory", events.getNamespace());
	}
	
	/**
	 * @return all event category names
	 */
	public List getEventCategories() {
		Element events = doc.getRootElement().getChild("events", doc.getRootElement().getNamespace());
		List eventCategories = events.getChildren("eventCategory", events.getNamespace());
		List eventCategorieNames = new ArrayList();
		for (int i = 0; i < eventCategories.size(); i++) {
			Element eventCategory = (Element) eventCategories.get(i);
			eventCategorieNames.add(eventCategory.getAttributeValue("NAME"));
		}
		return eventCategorieNames;
	}
	
	/**
	 * @param category the category from which all events should be
	 * @return all event elements
	 */
	public List getEvents(String category) {
		Element events = doc.getRootElement().getChild("events", doc.getRootElement().getNamespace());
		List eventCategories = events.getChildren("eventCategory", events.getNamespace());
		List eventsList = null;
		for (int i = 0; i < eventCategories.size(); i++) {
			Element cat = (Element) eventCategories.get(i);
			if (cat.getAttributeValue("NAME").equals(category)) {
				eventsList = cat.getChildren("event", doc.getRootElement().getNamespace());
			}
		}
		return eventsList;
	}
	
	/**
	 * @param categoryName the event category name which should be added
	 */
	public void addEventCategory(String categoryName) {
		Element evCat = new Element("eventCategory", doc.getRootElement().getNamespace());
		evCat.setAttribute("NAME", categoryName);
		Element events = doc.getRootElement().getChild("events", doc.getRootElement().getNamespace());
		events.addContent(evCat);
		writeTree();
	}
	
	/**
	 * @param category the event category to which the event should be added
	 * @param comment the comment to this event
	 * @param event the name of this event
	 * @param id the id of this event
	 */
	public void addEvent(String category, String comment, String event, String id) {
		Element ev = new Element("event", doc.getRootElement().getNamespace());
		ev.setAttribute("NAME", event);
		ev.setAttribute("ID", id);
		Comment com = new Comment(comment);
		ev.addContent(com);
		Element events = doc.getRootElement().getChild("events", doc.getRootElement().getNamespace());
		List eventCategories = events.getChildren("eventCategory", events.getNamespace());
		for (int i = 0; i < eventCategories.size(); i++) {
			Element evCat = (Element) eventCategories.get(i);
			if (evCat.getAttributeValue("NAME").equals(category)) {
				evCat.addContent(ev);
				break;
			}
		}
		eventIdMap.put(id, event);
		eventNameMap.put(event, id);
		eventCategoryMap.put(event, category);
		writeTree();
	}
	
	/**
	 * @param id the id of the searched event name
	 * @return the name according to this event id
	 */
	public String getEventName(String id) {
		if (eventIdMap.containsKey(id)) {
			return eventIdMap.get(id).toString();
		}
		return id;
	}
	
	/**
	 * @param eventName the name of the searched event id
	 * @return the id according to this event name
	 */
	public String getEventId(String eventName) {
		if (eventNameMap.containsKey(eventName)) {
			return eventNameMap.get(eventName).toString();
		}
		return null;
	}
	
	/**
	 * @param eventName the name of the event to which the category should be found
	 * @return the category according to this event name
	 */
	public String getEventCategoryName(String eventName) {
		if (eventCategoryMap.containsKey(eventName)) {
			return eventCategoryMap.get(eventName).toString();
		}
		return null;
	}
	
	/**
	 * @return all action category names
	 */
	public List getActionCategories() {
		Element actions = doc.getRootElement().getChild("actions", doc.getRootElement().getNamespace());
		List actionCategories = actions.getChildren("actionCategory", actions.getNamespace());
		List categorieNames = new ArrayList();
		for (int i = 0; i < actionCategories.size(); i++) {
			Element actionCategory = (Element) actionCategories.get(i);
			categorieNames.add(actionCategory.getAttributeValue("NAME"));
		}
		return categorieNames;
	}
	
	/**
	 * @return all action category elements
	 */
	public List getAllActionCategories() {
		Element actions = doc.getRootElement().getChild("actions", doc.getRootElement().getNamespace());
		return actions.getChildren("actionCategory", actions.getNamespace());
	}
	
	/**
	 * @param category the category from which all actions should be
	 * @return all action elements
	 */
	public List getActions(String category) {
		Element actions = doc.getRootElement().getChild("actions", doc.getRootElement().getNamespace());
		List actionCategories = actions.getChildren("actionCategory", actions.getNamespace());
		List actionsList = null;
		for (int i = 0; i < actionCategories.size(); i++) {
			Element acCat = (Element) actionCategories.get(i);
			if (acCat.getAttributeValue("NAME").equals(category)) {
				actionsList = acCat.getChildren("action", doc.getRootElement().getNamespace());
			}
		}
		return actionsList;
	}
	
	/**
	 * @param categoryName the action category name which should be added
	 */
	public void addActionCategory(String categoryName) {
		Element acCat = new Element("actionCategory", doc.getRootElement().getNamespace());
		acCat.setAttribute("NAME", categoryName);
		Element actions = doc.getRootElement().getChild("actions", doc.getRootElement().getNamespace());
		actions.addContent(acCat);
		writeTree();
	}
	
	/**
	 * @param category the action category to which the action should be added
	 * @param comment the comment to this action
	 * @param action the name of this action
	 * @param id the id of this action
	 */
	public void addAction(String category, String comment, String action, String id) {
		Element ac = new Element("action", doc.getRootElement().getNamespace());
		ac.setAttribute("NAME", action);
		ac.setAttribute("ID", id);
		actionIdMap.put(id, action);
		actionNameMap.put(action, id);
		actionCategoryMap.put(action, category);
		if (!(comment == null || comment == "")) {
			Comment com = new Comment(comment);
			ac.addContent(com);
		}
		Element actions = doc.getRootElement().getChild("actions", doc.getRootElement().getNamespace());
		List actionCategories = actions.getChildren("actionCategory", actions.getNamespace());
		for (int i = 0; i < actionCategories.size(); i++) {
			Element acCat = (Element) actionCategories.get(i);
			if (acCat.getAttributeValue("NAME").equals(category)) {
				acCat.addContent(ac);
				break;
			}
		}
		writeTree();
	}
	
	/**
	 * @param id the id of the searched action name
	 * @return the name according to this action id
	 */
	public String getActionName(String id) {
		if (actionIdMap.containsKey(id)) {
			return actionIdMap.get(id).toString();
		}
		return id;
	}
	
	/**
	 * @param actionName the name of the searched action id
	 * @return the id according to this action name
	 */
	public String getActionId(String actionName) {
		if (actionNameMap.containsKey(actionName)) {
			return actionNameMap.get(actionName).toString();
		}
		return null;
	}
	
	/**
	 * @param actionName the name of the action to which the category should be found
	 * @return the category according to this action name
	 */
	public String getActionCategoryName(String actionName) {
		if (actionCategoryMap.containsKey(actionName)) {
			return actionCategoryMap.get(actionName).toString();
		}
		return null;
	}
	
	/**
	 * @param id the searched event id
	 * @return true if the event id is already in use, false otherwise
	 */
	public boolean containsEventId (String id) {
		if (eventIdMap.containsKey(id)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param id the searched event name
	 * @return true if the event name is already in use, false otherwise
	 */
	public boolean containsEventName (String eventName) {
		if (eventNameMap.containsKey(eventName)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param id the searched event category
	 * @return true if the event category is already in use, false otherwise
	 */
	public boolean containsEventCategory (String eventCategory) {
		if (allEventCats.contains(eventCategory)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param id the searched action id
	 * @return true if the action id is already in use, false otherwise
	 */
	public boolean containsActionId (String id) {
		if (actionIdMap.containsKey(id)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param actionName the searched action name
	 * @return true if the action name is already in use, false otherwise
	 */
	public boolean containsActionName (String actionName) {
		if (actionNameMap.containsKey(actionName)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param actionCategory the searched action category
	 * @return true if the action category is already in use, false otherwise
	 */
	public boolean containsActionCategory (String actionCategory) {
		if (allActionCats.contains(actionCategory)) {
			return true;
		}
		return false;
	}
}