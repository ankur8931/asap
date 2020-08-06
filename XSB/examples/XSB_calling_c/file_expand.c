#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <pwd.h>
#include "cinterf.h"
/* context.h is necessary for the type of a thread context. */
#include "context.h"

/* 
 * Expands the initial ~ of a Unix filename and returns the absolute 
 * file name.  Otherwise it returns the file name unchanged.
 */

#ifdef MULTI_THREAD
	th_context *th = NULL;
#define xsb_get_main_thread_macro() xsb_get_main_thread()
#else
#define xsb_get_main_thread_macro() 
#endif

DllExport int call_conv expand_file()
{
char *file_name;
char *expanded_file_name;

int tlen, lose;
struct passwd *pw;
register char *new_dir, *p, *user_name;

#ifdef MULTI_THREAD
if( NULL == th) th = xsb_get_main_thread();
#endif

file_name = extern_ptoc_string(1);

/* If file_name is absolute, flush ...// and detect /./ and /../.
If no /./ or /../ we can return right away. */

if (file_name[0] == '/')
	{
	p = file_name; lose = 0;
	while (*p)
		{
		if (p[0] == '/' && p[1] == '/')
			file_name = p + 1;
		if (p[0] == '/' && p[1] == '~')
			file_name = p + 1, lose = 1;
		if (p[0] == '/' && p[1] == '.'
				&& (p[2] == '/' || p[2] == 0
				|| (p[2] == '.' && (p[3] == '/' || p[3] == 0))))
			lose = 1;
		p++;
		}
	if (!lose)
		{
		ctop_string(CTXTc 2, file_name);
		return TRUE;
		}
	}

/* Now determine directory to start with and put it in new_dir */

new_dir = 0;

if (file_name[0] == '~' && file_name[1] == '/')	/* prefix  ~/ */
	{
	if (!(new_dir = (char *) getenv("HOME")))
		new_dir = "";
	file_name++;
	}
else		/* prefix  ~username/ */
	{
	for (p = file_name; *p && (*p != '/'); p++);
		user_name = alloca(p - file_name + 1);
	bcopy (file_name, user_name, p - file_name);
	user_name[p - file_name] = 0;
	
	pw = (struct passwd *) getpwnam(user_name + 1);
	if (!pw)
		{
		fprintf(stderr, "++Error: \"%s\" is not a registered user\n", user_name + 1);
		ctop_string(CTXTc 2, file_name); /* return the input file name unchanged */
		return TRUE;
		}
	
	file_name = p;
	new_dir = pw -> pw_dir;
	}

/* Now concatenate the directory and name to new space in the stack frame */

tlen = (new_dir ? strlen(new_dir) + 1 : 0) + strlen(file_name) + 1;
expanded_file_name = alloca(tlen);
if (new_dir) strcpy(expanded_file_name, new_dir);
	strcat(expanded_file_name, file_name);


/* Make sure you insert the newly created symbol into the symbol table. */

ctop_string(CTXTc 2, string_find(expanded_file_name, 1));

return TRUE;
}




