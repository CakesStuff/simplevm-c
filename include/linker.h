#ifndef _LINKER_H
#define _LINKER_H
#include<core.h>
#include<args.h>

typedef enum {
    RELOCATION_INVALID,
    RELOCATION_UPPER12_LOWER12,
    RELOCATION_LOWER4_LOWER4,
} LinkRelocationType;

typedef enum {
    LABEL_UNDEFINED,
    LABEL_ADDRESS,
    LABEL_ABSVAL,
} LinkLabelType;

typedef struct {
    uint8_t name[32];
    uint8_t type;
    uint64_t value;
} __attribute__((packed)) LinkLabel;

typedef struct {
    uint64_t label_id;
    uint8_t type;
    uint64_t offset;
} __attribute__((packed)) LinkRelocation;

typedef struct {
    uint64_t label_count;
    LinkLabel* labels;
    uint64_t relocation_count;
    LinkRelocation* relocations;
    uint64_t size;
    uint8_t* content;
} LinkFile;

/**
 * Loads a linkfile struct from a buffer
 * 
 * \param content the content of the buffer
 * \param size the size of the buffer
 * \returns A LinkFile handle or NULL on failure.
 */
LinkFile* link_from_buffer(uint8_t* content, size_t size);

/**
 * Writes a linkfile struct to a buffer
 * 
 * \param lf the linkfile handle
 */
void link_to_buffer(LinkFile* lf, uint8_t* content);

/**
 * Destroys a link file handle and frees it.
 * 
 * \param lf the link file handle
 */
void link_file_destroy(LinkFile* lf);

/**
 * Links two files together without resolving relocations.
 * 
 * \param ins handle to the vector of inputs, is needed because it needs to be modified
 * \param file_1 index of file_1
 * \param file_2 index of file_2
 * \returns True on success.
 */
bool link_together(inp_v* ins, size_t file_1, size_t file_2);

/**
 * Resolves all relocations and produces the final binary.
 * 
 * \param in the file to resolve
 * \returns True on success.
 */
bool link_resolve(CCInput* in);

#endif /* _LINKER_H */