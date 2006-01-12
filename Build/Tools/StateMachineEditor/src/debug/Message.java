package debug;

public class Message {
	private String name;
	private String instanceAddress;
	
	
	public Message() {
		super();
	}
	
	public void setInstanceAddress(String instanceAddress) {
		this.instanceAddress = instanceAddress;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getInstanceAddress () {
		return this.instanceAddress;
	}
	
	public String getName() {
		return this.name;
	}

}
