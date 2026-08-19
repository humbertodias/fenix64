/*-------------------------------------------------------------------------------------------------------
Copyright (c) 2003 Joseba García Etxebarria, under the GPL, as described
on copying in the (Fenix)/COPYING file.
This file provides a thin BeOS->C compatibility layer, as the BeOS kits are
designed for C++.
Now alert dialogs are created using the system call, but this may change as to
be able to use it in fullscreen modes (writting my own implementation of the
dialog using BDirectWindow)
We don't need to create a BApplication objet, SDL does that for us.
-------------------------------------------------------------------------------------------------------*/
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <be/interface/Alert.h>

extern "C" __declspec(dllexport) void be_alert(const char *error) {
	char *error_msg;
	int len;
	error_msg = strdup(error);

	for (len = strlen(error_msg); len && (error_msg[len-1] == '\n'); --len)
		error_msg[len-1] = 0;

	BAlert * alert_p = new BAlert("FXI Error", error_msg, "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert_p -> Go();
	free (error_msg);
}
