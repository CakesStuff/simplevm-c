#ifndef _FILES_H
#define _FILES_H
#include<core.h>
#include<args.h>

/**
 * Loads the file contents of a file into the buffer
 * 
 * \param in handle to the ccinput
 * \returns True on success.
 */
bool file_load(CCInput* in);

/**
 * Handles the special case of "-" as stdin file
 * 
 * \param in handle to the ccinput
 * \returns True on success.
 */
bool file_load_stdin(CCInput* in);

#endif /* _FILES_H */