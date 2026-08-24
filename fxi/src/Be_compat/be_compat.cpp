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
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <be/kernel/OS.h>
#include <be/interface/Alert.h>
#include <be/app/Clipboard.h>
#include <be/app/Application.h>

//Don't use with Fenix, or it'll segfault at runtime
extern "C" __declspec(dllexport) void be_application(const char *signature) {
	new BApplication(signature);
}

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

//Clipboard operations
extern "C" __declspec(dllexport) void be_copy_raw(const char *mime_type, const void *data, ssize_t numbytes) {

	BMessage *clip = (BMessage *)NULL;
	
	if(be_clipboard->Lock()) {
		be_clipboard->Clear();
		if((clip = be_clipboard->Data())) {
			clip->AddData(mime_type, B_MIME_TYPE, data, numbytes);
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
	}
}

extern "C" __declspec(dllexport) const void *be_paste_raw(const char *mime_type) {

	BMessage *clip = (BMessage *)NULL;
	const void *data;
	int32 numbytes;
	
		if(be_clipboard->Lock()) {
			if((clip = be_clipboard->Data())) 
				clip->FindData(mime_type, B_MIME_TYPE, (const void **)&data, &numbytes);
			be_clipboard->Unlock();
		}
	return data;
}

extern "C" __declspec(dllexport) void be_copy_text(const char *text) {
	be_copy_raw("text/plain", text, strlen(text));
}

extern "C" __declspec(dllexport) const char *be_paste_text() {
	return (const char *)be_paste_raw("text/plain");	//Doesn't check fro errors
}

