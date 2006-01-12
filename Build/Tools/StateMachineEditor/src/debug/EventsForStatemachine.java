package debug;

import java.util.ArrayList;
import java.util.List;

public class EventsForStatemachine {

	private List eventList = new ArrayList();
	
	public EventsForStatemachine() {
		super();
	}

	public void addEvent(String eventId) {
		eventList.add(eventId);
	}
	
	public List getEvents() {
		return eventList;
	}
	
}
