#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>

void read_from_property(Display*, Window, Atom, Atom);

int main(int argc, char* argv[]) {	
	
	Display * xdpy;
	Window client_window, root_window, owner_window;
	int screen;
	Atom prop_name, prop_type, selection;
	XEvent event;
	XSelectionEvent selection_event;
	
	char * selection_name = NULL;
	char * property_name = "WRITE_TO_HERE";
	char * property_type = "UTF8_STRING";
	
	if (argc < 2) {
		selection_name = "CLIPBOARD";
	} else if ( !strcmp(argv[1], "CLIPBOARD") || !strcmp(argv[1], "PRIMARY") ) {
		selection_name = argv[1];
	} else {
		selection_name = "CLIPBOARD";
	}
	
	xdpy = XOpenDisplay(NULL);
	if(!xdpy) {
		printf("Couldn't connect to the X Server\n");
		return 1;
	}
	
	screen = DefaultScreen(xdpy);
	root_window = RootWindow(xdpy, screen);
	client_window = XCreateSimpleWindow(xdpy, root_window, -10, -10, 1, 1, 0, 0, 0);

	selection = XInternAtom(xdpy, selection_name, False);
	prop_name = XInternAtom(xdpy, property_name, False);
	prop_type = XInternAtom(xdpy, property_type, False);
	if (!selection || !prop_name || !prop_type) {
		printf("Failed to create Atoms\n");
		return 1;
	}
	
	owner_window = XGetSelectionOwner(xdpy, selection);
	if (!owner_window) {
		printf("\"%s\" Selection has no owner", selection_name);
		return 1;
	}
	
	// request to convert the Selection content to UTF8 and write it to our property
	XConvertSelection(xdpy, selection, prop_type, prop_name, client_window, CurrentTime);
	
	// block waiting for the selection content
	while (1) {
		// read the next event from the Event Queue if nothing found wait until an event is recieved
		XNextEvent(xdpy, &event);
		// check the event type
		switch (event.type) {
			// content is written to the property
			case SelectionNotify:
				selection_event = (XSelectionEvent)event.xselection;

				if (!selection_event.send_event) {
					printf("Selection \"%s\" has no owner\n", selection_name);
					return 1;
				} else if (selection_event.property == None) {
					printf("Owner couldn't convert Selection \"%s\" content to the type \"%s\" \n", \
					selection_name, property_type);
					return 1;
				} else {
					read_from_property(xdpy, client_window, selection_event.property, prop_type);
					return 0;
				}
				
			default:
				break;
		}
	}
	
	return 1;
}

void read_from_property(Display * xdpy, Window target_window, Atom prop_name, Atom prop_type) {
	/*
		int XGetWindowProperty(display, w, property, long_offset, long_length, delete, req_type, 
				        actual_type_return, actual_format_return, nitems_return, bytes_after_return, 
				        prop_return)
	*/
	// r_* vars will be written to by XGetWindowProperty()
	unsigned long size_to_retrieve, from_offset;
	Bool delete_after_read;
	unsigned long r_number_of_items;
	Atom r_type_of_prop;
	int r_format_of_prop;
	unsigned char * r_retrieved_data = NULL;
	unsigned long r_remaining_data_size;
	
	// double call XGetWindowProperty() to avoid reading the data partially
	// first call to get the size of data in r_remaining_data_size
	// second call to read the whole data
	
	// we first call XGetWindowProperty() to get the size of data in r_retrieved_data_size
	from_offset = 0; // retrieve from offest 0
	delete_after_read = False;
	size_to_retrieve = 0; // retrieve 0 bytes
	XGetWindowProperty(xdpy, target_window, prop_name, from_offset, size_to_retrieve, delete_after_read, prop_type, &r_type_of_prop, &r_format_of_prop, &r_number_of_items, &r_remaining_data_size, &r_retrieved_data);
	 
	XFree(r_retrieved_data); // free the allocation done by XGetWindowProperty()
	
	// read the whole data
	size_to_retrieve = r_remaining_data_size;
	XGetWindowProperty(xdpy, target_window, prop_name, from_offset, size_to_retrieve, delete_after_read, prop_type, &r_type_of_prop, &r_format_of_prop, &r_number_of_items, &r_remaining_data_size, &r_retrieved_data);
	printf("%s", r_retrieved_data);
	XFree(r_retrieved_data); // free the allocation done by XGetWindowProperty()

}
