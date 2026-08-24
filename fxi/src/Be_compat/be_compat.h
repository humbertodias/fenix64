#ifndef _BE_COMPAT_H
#define _BE_COMPAT_H

void be_application(const char *signature);		//Don't use together with SDL, app signatures are changed through the resource file
void be_alert (const char *error);
void be_copy_raw(const char *mime_type, const void *data, ssize_t numbytes);
const void *be_paste_raw(const char *mime_tpye);
void be_copy_text (const char *text);
const char *be_paste_text ();
#endif
