package dialog;

import java.awt.Component;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JDialog;
import javax.swing.plaf.basic.BasicComboBoxRenderer;

import model.Connection;

import org.jdom.Comment;
import org.jdom.Element;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

public class EventDialog extends JDialog {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private javax.swing.JPanel jContentPane = null;
	private JLabel eventCategoryLabel = null;
	private JComboBox eventCategoryComboBox = null;
	private JButton newCategoryButton = null;
	private JButton oKButton = null;
	private JButton cancelButton = null;
	private ActionAndEventInfo info;
	private JLabel eventNameLabel = null;
	private JComboBox eventNameComboBox = null;
	private JButton newEventButton = null;
	private String event = null;
	private String category;
	private List tooltips = new ArrayList();
	private ReadActionAndEventDOMTree eventTree = null;
	
	/**
	 * This is the default constructor
	 */
	public EventDialog() {
		super();
		initialize();
	}
	
	/**
	 * constructor to change an event
	 * @param con the connection to which the event belongs
	 */
	public EventDialog(Connection con) {
		super();
		this.event = con.getEvent();
		initialize();
		
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		eventTree = DOMGetInstance.getActionAndEventInstance();
		this.setSize(680, 200);
		this.setName("ChangeEventDialog");
		this.setTitle("Change event");
		this.setLocationRelativeTo(null);
		this.setResizable(false);
		this.setModal(true);
		this.setAlwaysOnTop(true);
		this.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
		this.setContentPane(getJContentPane());
	}

	/**
	 * This method initializes jContentPane
	 * 
	 * @return javax.swing.JPanel
	 */
	private javax.swing.JPanel getJContentPane() {
		if(jContentPane == null) {
			eventNameLabel = new JLabel();
			eventNameLabel.setBounds(new java.awt.Rectangle(30,40,120,20));
			eventNameLabel.setText("Event name:");
			eventCategoryLabel = new JLabel();
			eventCategoryLabel.setBounds(new java.awt.Rectangle(30,10,120,20));
			eventCategoryLabel.setText("Event category:");
			jContentPane = new javax.swing.JPanel();
			jContentPane.setLayout(null);
			jContentPane.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
			jContentPane.add(eventCategoryLabel, null);
			jContentPane.add(getEventCategoryComboBox(), null);
			jContentPane.add(getNewCategoryButton(), null);
			jContentPane.add(getNewEventButton(), null);
			jContentPane.add(getOKButton(), null);
			jContentPane.add(getCancelButton(), null);
			jContentPane.add(eventNameLabel, null);
			jContentPane.add(getEventNameComboBox(), null);
		}
		return jContentPane;
	}

	/**
	 * This method initializes jComboBox	
	 * 	
	 * @return javax.swing.JComboBox	
	 */
	private JComboBox getEventCategoryComboBox() {
		if (eventCategoryComboBox == null) {
			eventCategoryComboBox = new JComboBox();
			eventCategoryComboBox.addItemListener(new java.awt.event.ItemListener() {
				public void itemStateChanged(java.awt.event.ItemEvent e) {
					category = eventCategoryComboBox.getSelectedItem().toString();
					if (!(eventNameComboBox == null)) {
						eventNameComboBox.removeAllItems();
						List allEvents = eventTree.getEvents(category);
						tooltips.clear();
						for (int i = 0; i < allEvents.size(); i++) {
							Element ev = (Element) allEvents.get(i);
							String event = ev.getAttributeValue("NAME");
							eventNameComboBox.addItem(event);
							List commentList = ev.getContent();
							// set comments for each category to the tooltip
							String toolTip = "";
							for (int j = 0; j < commentList.size(); j++) {
								Object comEle = commentList.get(j);
								if (comEle instanceof Comment) {
									Comment comment = (Comment) comEle;
									toolTip = comment.getText();
									break;
								}
							}
							if (toolTip.equals("")) {
								toolTip = "No comment available";
							}
							tooltips.add(toolTip);
						}
						eventNameComboBox.setRenderer(new MyComboBoxRenderer());
					}
				}
			});
			eventCategoryComboBox.setMaximumRowCount(5);
			List allCategories = eventTree.getEventCategories();
			for (int i = 0; i < allCategories.size(); i++) {
				eventCategoryComboBox.addItem(allCategories.get(i).toString());
			}
			eventCategoryComboBox.setBounds(new java.awt.Rectangle(160,10,360,20));
			if (!(eventTree.getEventCategoryName(event) == null)) {
				eventCategoryComboBox.setSelectedItem(eventTree.getEventCategoryName(event));
			}
		}
		return eventCategoryComboBox;
	}

	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getNewCategoryButton() {
		if (newCategoryButton == null) {
			newCategoryButton = new JButton();
			newCategoryButton.setBounds(new java.awt.Rectangle(530,10,120,20));
			newCategoryButton.setText("New category");
			newCategoryButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					while (true) {
						String inputValue = JOptionPane.showInputDialog(EventDialog.this, "New category name:");
						if (inputValue == null) {
							//cancel
							break;
						}
						else if (!(inputValue == null) && !inputValue.equals("") && !eventTree.containsEventCategory(inputValue)) {
							Object[] options = { "OK", "CANCEL" };
							int option = JOptionPane.showOptionDialog(EventDialog.this, "Do you really want to add this category?\nAdded categories can not be removed!", "Click OK to continue",
										JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE,
										null, options, options[0]);
							if (option == 0) {
								eventTree.addEventCategory(inputValue);
								eventCategoryComboBox.addItem(inputValue);
								eventCategoryComboBox.setSelectedItem(inputValue);
							}
							break;
						}
						else if (eventTree.containsEventCategory(inputValue)) {
							JOptionPane.showMessageDialog(EventDialog.this,
									"Event category already exists, please choose another one!", "Error Message",
									JOptionPane.ERROR_MESSAGE);
						}
						else {
							JOptionPane.showMessageDialog(EventDialog.this,
									"You have to enter a category name!", "Error Message",
									JOptionPane.ERROR_MESSAGE);
						}
					}
				}
			});
		}
		return newCategoryButton;
	}

	/**
	 * This method initializes jButton1	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getOKButton() {
		if (oKButton == null) {
			oKButton = new JButton();
			oKButton.setBounds(new java.awt.Rectangle(480,100,80,30));
			oKButton.setText("OK");
			oKButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					if (eventNameComboBox.getSelectedItem() == null) {
						JOptionPane.showMessageDialog(EventDialog.this,
								"You have to select an event name!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					info = new ActionAndEventInfo(null, eventNameComboBox.getSelectedItem().toString(), null);
					cancel();
				}
			});
		}
		return oKButton;
	}
	
	/**
	 * This method initializes jButton2	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getCancelButton() {
		if (cancelButton == null) {
			cancelButton = new JButton();
			cancelButton.setBounds(new java.awt.Rectangle(570,100,80,30));
			cancelButton.setText("Cancel");
			cancelButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					cancel();
				}
			});
		}
		return cancelButton;
	}
	
	/**
	 * disposes this dialog
	 */
	void cancel() {
		dispose();
	}
	
	/**
	 * @return the info object with comment and name
	 */
	public ActionAndEventInfo getInfo() {
		return info;
	}

	/**
	 * This method initializes jComboBox	
	 * 	
	 * @return javax.swing.JComboBox	
	 */
	private JComboBox getEventNameComboBox() {
		boolean contains = false;
		if (eventNameComboBox == null) {
			eventNameComboBox = new JComboBox();
			eventNameComboBox.setMaximumRowCount(5);
			List allEvents = eventTree.getEvents(category);
			for (int i = 0; i < allEvents.size(); i++) {
				Element ev = (Element) allEvents.get(i);
				String event = ev.getAttributeValue("NAME");
				eventNameComboBox.addItem(event);
				List commentList = ev.getContent();
				// set comments for each event to the tooltip
				String toolTip = "";
				for (int j = 0; j < commentList.size(); j++) {
					Object comEle = commentList.get(j);
					if (comEle instanceof Comment) {
						Comment comment = (Comment) comEle;
						toolTip = comment.getText();
						break;
					}
				}
				if (toolTip.equals("")) {
					toolTip = "No comment available";
				}
				tooltips.add(toolTip);
			}
			if (!(event == null)) {
				for (int i = 0; i < eventNameComboBox.getItemCount(); i++) {
					String eventName = (String) eventNameComboBox.getItemAt(i);
					if (eventName.equals(event)) {
						contains = true;
						break;
					}
				}
				if (contains == false) {
					eventNameComboBox.addItem(event);
					tooltips.add("No comment available");
				}
				eventNameComboBox.setSelectedItem(event);
			}
			eventNameComboBox.setRenderer(new MyComboBoxRenderer());
			eventNameComboBox.setBounds(new java.awt.Rectangle(160,40,360,20));
		}
		return eventNameComboBox;
	}
	
	/**
	 * @author Daniel
	 * adds a tooltip to each combobox entry
	 */
	class MyComboBoxRenderer extends BasicComboBoxRenderer {
		private static final long serialVersionUID = 1L;
		
		public Component getListCellRendererComponent( JList list, 
				Object value, int index, boolean isSelected, boolean cellHasFocus) {
			if (isSelected) {
				setBackground(list.getSelectionBackground());
				setForeground(list.getSelectionForeground());      
				if (-1 < index) {
					list.setToolTipText(tooltips.get(index).toString());
				}
			} else {
				setBackground(list.getBackground());
				setForeground(list.getForeground());
			} 
			setFont(list.getFont());
			setText((value == null) ? "" : value.toString());     
			return this;
		}  
	}
	
	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getNewEventButton() {
		if (newEventButton == null) {
			newEventButton = new JButton();
			newEventButton.setBounds(new java.awt.Rectangle(530,40,120,20));
			newEventButton.setText("New event");
			newEventButton.setActionCommand("New event");
			newEventButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					NewEventDialog evDlg = new NewEventDialog(EventDialog.this);
					evDlg.setVisible(true);
					String newComment = evDlg.getEventComment();
					String newName = evDlg.getEventName();
					String newId = evDlg.getEventId();
					if (!evDlg.isCanceled()) {
						if (!(newName == null || newId == null) && !(newName.equals("") || newId.equals(""))) {
							Object[] options = { "OK", "CANCEL" };
							int option = JOptionPane.showOptionDialog(EventDialog.this, "Do you really want to add this event?\nAdded events can not be removed!", "Click OK to continue",
										JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE,
										null, options, options[0]);
							if (option == 0) {
								eventTree.addEvent(eventCategoryComboBox.getSelectedItem().toString(), newComment, newName, newId);
								eventNameComboBox.addItem(newName);
								if (!newComment.equals("")) {
									tooltips.add(newComment);
								}
								else {
									tooltips.add("No comment available");
								}
								eventNameComboBox.setSelectedItem(newName);
							}
						}
					}
				}
			});
		}
		return newEventButton;
	}
}
