package router;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class MyMultiValueMap {
	private HashMap map = new HashMap();
	
	public MyMultiValueMap() { super(); }
	
	public ArrayList get(Object key) {
		Object value = map.get(key);
		if (value == null) return null;
		
		if (value instanceof ArrayList) 
			return (ArrayList) value;
		ArrayList v = new ArrayList(1);
		v.add(value);
		return v;
	}
	
	
	
	public void put(Object key, Object value) {
		Object existingValues = map.get(key);
		if (existingValues == null) {
			map.put(key, value);
			return;
		}
		if (existingValues instanceof ArrayList) {
			ArrayList v = (ArrayList) existingValues;
			if (!v.contains(value))
				v.add(value);
			return;
		}
		if (existingValues != value) {
			ArrayList v = new ArrayList(2);
			v.add(existingValues);
			v.add(value);
			map.put(key, v);
		}
	}
	
	public int remove(Object key, Object value) {
		Object existingValues = map.get(key);
		if (existingValues != null) {
			if (existingValues instanceof ArrayList) {
				ArrayList v = (ArrayList) existingValues;
				int result = v.indexOf(value);
				if (result == -1)
					return -1;
				v.remove(result);
				if (v.isEmpty())
					map.remove(key);
				return result;
			}
			if (map.remove(key) != null)
				return 0;
		}
		return -1;
	}
	
	public Object removeValue(Object value) {
		Iterator iter = map.values().iterator();
		Object current;
		while (iter.hasNext()) {
			current = iter.next();
			if (value.equals(current)) {
				iter.remove();
				return value;
			} else if (current instanceof List) {
				if (((List)current).remove(value)) {
					if (((List)current).isEmpty())
						iter.remove();
					return value;
				}
			}
		}
		return null;
	}
	
	public int size() {
		return map.size();
	}
}