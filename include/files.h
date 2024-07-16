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

/**
 * Changes the file ending to the appropriate one depending on the file type.
 * If no ending is present, it adds the ending.
 */
void file_adjust_ending(CCInput* in);

/**
 * Writes a file to path.
 * 
 * \param in the file to write
 * \param path where to write the file
 */
bool file_write(CCInput* in, char* file);

#endif /* _FILES_H */