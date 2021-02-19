// compile: gcc ply-notify.c $(pkg-config --libs --cflags dbus-1) -o ply-notify
// run: ply-notify "PLY" "PLY has started"

// we cannot use "dbus-send" to deliver our message, because "Notify" method has a required argument of type "dict of variant" :
// <arg type="a{sv}" name="arg_6" direction="in"> </arg> // arg_6 = hints
// and "dbus-send" does not support this type of arguments, so we have to use our own program
// Note: "gdbus" does support this type of datatype.

#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>

#define OBJECT "/org/freedesktop/Notifications"
#define INTERFACE "org.freedesktop.Notifications"
#define METHOD "Notify"


DBusError dbus_error; // error object/stucture

dbus_bool_t build_message(DBusMessage*, char *, char *, char * );
void print_error_and_exit();

int main(int argc, char* argv[]) {
	int name; // our name on the BUS
	DBusMessage * dbus_message; // DBUS message object
	DBusConnection * dbus_connection; // DBUS connection object

	dbus_error_init(&dbus_error); // init the error object
	
        dbus_connection = dbus_bus_get(DBUS_BUS_SESSION, &dbus_error); // connect to the session bus
        if (!dbus_connection)
        	print_error_and_exit();

	name = dbus_bus_request_name(dbus_connection, "io.ply.callyou", DBUS_NAME_FLAG_REPLACE_EXISTING, &dbus_error);
	if (!name)
		print_error_and_exit();


	dbus_message = dbus_message_new_method_call(NULL, OBJECT, INTERFACE, METHOD);
	if (!dbus_message)
		print_error_and_exit();
	
	if ( !dbus_message_set_destination(dbus_message, INTERFACE); ) {
		fprintf(stderr, "Error while setting destination: Out Of Memory");
                return 1;
	}
	
	if ( !build_message(dbus_message, argv[1], argv[1], argv[2]) ) {
		fprintf(stderr, "Error while building message: Out Of Memory");
                return 1;
	}
        if (!dbus_message)
		print_error_and_exit();

	// send message and block until reply (for a default timeout)
	// i could use dbus_pending_call_set_notify() with a callback but :P
	// dbus_message holds now the reply (NULL for error)
	dbus_message = dbus_connection_send_with_reply_and_block(dbus_connection, dbus_message, -1, &dbus_error);
	if (!dbus_message)
		print_error_and_exit();

 	// unref and return
	dbus_message_unref(dbus_message);
	dbus_connection_unref(dbus_connection);
	return 0;
} 

dbus_bool_t build_message(DBusMessage* message, char *app_name, char *summary, char *body) {
	/*
	Notify method signature:

	UINT32 org.freedesktop.Notifications.Notify ( 	<- <arg type="u" name="arg_8" direction="out"></arg> (return value)
	    STRING app_name,      			-> <arg type="s" name="arg_0" direction="in"></arg>
	    UINT32 replaces_id,  			-> <arg type="u" name="arg_1" direction="in"></arg>
	    STRING app_icon,     			-> <arg type="s" name="arg_2" direction="in"></arg>
	    STRING summary,     			-> <arg type="s" name="arg_3" direction="in"></arg>
	    STRING body,         			-> <arg type="s" name="arg_4" direction="in"></arg>
	    ARRAY actions,       			-> <arg type="as" name="arg_5" direction="in"></arg>
	    DICT hints,          			-> <arg type="a{sv}" name="arg_6" direction="in"></arg>
	    INT32 expire_timeout 			-> <arg type="i" name="arg_7" direction="in"></arg>
        );
	*/
	
	DBusMessageIter arguments;
	DBusMessageIter iterators[3];

	int replaces_id = 1337; // unsigned int
	char * app_icon = ""; // string
	char * actions[] = { NULL }; // array of strings
	char * hints_key = "foo"; // string
	char * hints_value = "bar"; // this actaully can be pretty much anything that can be wrapped in a variant
	signed int timeout = -1; // signed int

	dbus_bool_t success = 1;
	
	// init arguments appending
	dbus_message_iter_init_append(message, &arguments);
	
	// all these functions return TRUE on success and FALSE on failure
	// if any of these calls fails build_message will return FALSE

	// append basic params to arguments
   	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING, &app_name);
   	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_UINT32, &replaces_id);
   	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING, &app_icon);
   	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING, &summary);
   	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING, &body);
   
	// D-BUS docs:
	// To append variable-length basic types, or any more complex value, 
	// you have to use an iterator.

	// So here we go:
	// append actions[] (array of strings) :
	success &= dbus_message_iter_open_container(&arguments, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &iterators[0]);
	for (int i = 0; actions[i]; i++ )
		success &= dbus_message_iter_append_basic(&iterators[0], DBUS_TYPE_STRING, &actions[i]);
	success &= dbus_message_iter_close_container(&arguments, &iterators[0]);

	// append hints (dictionary datatype {"key":<"value">})
	success &= dbus_message_iter_open_container(&arguments, DBUS_TYPE_ARRAY, "{sv}", &iterators[0]); // open a container for dict {string: variant}
	
	success &= dbus_message_iter_open_container(&iterators[0], DBUS_TYPE_DICT_ENTRY, NULL, &iterators[1]); // open a container for the dict entry / key (type: string)
	success &= dbus_message_iter_append_basic(&iterators[1], DBUS_TYPE_STRING, &hints_key); // append "foo" (string)
	
	success &= dbus_message_iter_open_container(&iterators[1], DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &iterators[2]); // open a container for value (type: variant)
	success &= dbus_message_iter_append_basic(&iterators[2], DBUS_TYPE_STRING, &hints_value); // append "bar" (string)
	
	success &= dbus_message_iter_close_container(&iterators[1], &iterators[2]); // close opened containers
	success &= dbus_message_iter_close_container(&iterators[0], &iterators[1]);
	success &= dbus_message_iter_close_container(&arguments, &iterators[0]); 
	
        // append a basic param to arguments
	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_INT32, &timeout);

	return success;
}

void print_error_and_exit() {
	if (dbus_error_is_set(&dbus_error)) {
		fprintf(stderr, "Error: %s\n", dbus_error.message);
		dbus_error_free(&dbus_error); // free and re-init
		exit(1);
	}
	fprintf(stderr, "Unknown error occured\n");
	exit(1);
}
