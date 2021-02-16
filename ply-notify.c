// compile: gcc ply-notify.c $(pkg-config --libs --cflags dbus-1) -o ply-notify
// run: ply-notify "PLY" "PLY has started"
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define OBJECT "/org/freedesktop/Notifications"
#define INTERFACE "org.freedesktop.Notifications"
#define METHOD "Notify"

#define CHECK_IS_ERROR_SET 0xB
#define CHECK_CONNECTION 0xC
#define CHECK_IS_MSG_BUILT 0xE
#define CHECK_IS_MSG_SENT 0xD

DBusError dbus_error; // error object/stucture

dbus_bool_t build_message(DBusMessage*, char *, char *, char * );
dbus_bool_t check_for_error(char, void *);

int main(int argc, char* argv[]) {
	int name; // our name on the BUS
	DBusMessage * dbus_message; // DBUS message object
	DBusConnection * dbus_connection; // DBUS connection object

	dbus_error_init(&dbus_error); // init the error object
	dbus_connection = dbus_bus_get(DBUS_BUS_SESSION, &dbus_error); // connect to the session bus

	check_for_error(CHECK_CONNECTION, (void *)dbus_connection);

	name = dbus_bus_request_name(dbus_connection, "io.ply.callyou", DBUS_NAME_FLAG_REPLACE_EXISTING, &dbus_error);
	check_for_error( CHECK_IS_ERROR_SET, (void*)&dbus_error );


	dbus_message = dbus_message_new_method_call(NULL, OBJECT, INTERFACE, METHOD);
	check_for_error(CHECK_IS_MSG_BUILT, dbus_message);

	dbus_message_set_destination(dbus_message, INTERFACE);

	if ( !build_message(dbus_message, argv[1], argv[1], argv[2]) ) {
		fprintf(stderr, "Error while building message: Out of memory");
		return 1;
	}

	check_for_error(CHECK_IS_MSG_BUILT, dbus_message);

	// send message and block until reply (for a default timeout)
	// i could use dbus_pending_call_set_notify() with a callback but :P
	// *dbus_message holds now the reply (NULL for error)
	dbus_error_init(&dbus_error);
	dbus_message = dbus_connection_send_with_reply_and_block(dbus_connection, dbus_message, -1, &dbus_error);
	check_for_error(CHECK_IS_MSG_SENT, dbus_message);

 	// unref and return
	dbus_message_unref(dbus_message);
	dbus_connection_unref(dbus_connection);
	return 0;
} 


dbus_bool_t check_for_error(char check_type, void * object) {
	if (check_type == CHECK_IS_ERROR_SET) {
		if (dbus_error_is_set(&dbus_error)) {
			fprintf(stderr, "Connection Error (%s)\n", dbus_error.message);
			dbus_error_free(&dbus_error);
			exit(1);
		}
		return 0;

	}  else if (check_type == CHECK_CONNECTION) {
		object = (DBusConnection *)object;
		if (!object)
			exit(1);
		return 0;

   	}  else if (check_type == CHECK_IS_MSG_BUILT) {
      		object = (DBusMessage *)object;
      		if(!object)
         		exit(1);
      		return 0;

	}  else if (check_type == CHECK_IS_MSG_SENT) {
		object = (DBusMessage *)object;
		if (!object) {
			check_for_error(CHECK_IS_ERROR_SET, (void *)&dbus_error);
			exit(1);
		}
      		return 0;
 	}
	return 1; 
}


dbus_bool_t build_message(DBusMessage* message, char *app_name, char *summary, char *body) {
	/*
	Notify method signature:

	UINT32 org.freedesktop.Notifications.Notify (
		STRING app_name,
		UINT32 replaces_id, 
	    STRING app_icon, 
	    STRING summary, 
	    STRING body, 
	    ARRAY actions, 
	    DICT hints, 
	    INT32 expire_timeout
		);
	*/
	
	DBusMessageIter arguments;
	DBusMessageIter iterators[3];

	int replaces_id=1337;
	char *app_icon[]={""};
	char* actions[] = { NULL };
	char* hints_key = "key";
	int hints_value = *(int*)"value";
	int32_t timeout=-1;

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
	// you have to use an iterator rather than dbus_message_iter_append_basic()

	// So here we go:
	// append actions[] (array) :
	success &= dbus_message_iter_open_container(&arguments, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &iterators[0]);
	for (int i = 0; actions[i]; i++ )
		success &= dbus_message_iter_append_basic(&iterators[0], DBUS_TYPE_STRING, &actions[i]);
	success &= dbus_message_iter_close_container(&arguments,&iterators[0]);

	// append hints (dictionary datatype {"key":"value"})
	success &= dbus_message_iter_open_container(&arguments, DBUS_TYPE_ARRAY, "{sv}", &iterators[0]); // {sv} is the signature for dict type: {String, Variant}
	success &= dbus_message_iter_open_container(&iterators[0], DBUS_TYPE_DICT_ENTRY, NULL, &iterators[1]); 
	success &= dbus_message_iter_append_basic(&iterators[1] ,DBUS_TYPE_STRING, &hints_key); // dict entry is the key element in the dict
	success &= dbus_message_iter_open_container(&iterators[1] ,DBUS_TYPE_VARIANT, DBUS_TYPE_INT32_AS_STRING, &iterators[2]); // next is "value"
	success &= dbus_message_iter_append_basic(&iterators[2], DBUS_TYPE_INT32, &hints_value);
	success &= dbus_message_iter_close_container(&iterators[1], &iterators[2]);
	success &= dbus_message_iter_close_container(&iterators[0], &iterators[1]);
	success &= dbus_message_iter_close_container(&arguments, &iterators[0]); 
	
	success &= dbus_message_iter_append_basic(&arguments, DBUS_TYPE_INT32, &timeout);

	return success;
}