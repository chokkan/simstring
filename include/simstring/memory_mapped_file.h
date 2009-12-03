#ifndef __MEMORY_MAPPED_FILE_H__
#define __MEMORY_MAPPED_FILE_H__

#include <iostream>
#include <string>

class memory_mapped_file_base
{
public:
    typedef size_t size_type;

    memory_mapped_file_base() {}
    virtual ~memory_mapped_file_base() {}

    void open(const std::string& path, std::ios_base::openmode mode) {}
    bool is_open() const {return false; }
    void close() {}
    void resize(size_type size) {}
    size_type size() const {return 0; }
    char* data() const {return NULL; }
    const char* const_data() const {return NULL; }
    static int alignment() {return 0; }
};

#if     defined(_WIN32)
#include "memory_mapped_file_win32.h"
#define memory_mapped_file memory_mapped_file_win32

#else
#include "memory_mapped_file_posix.h"
#define memory_mapped_file memory_mapped_file_posix

#endif

#endif/*__MEMORY_MAPPED_FILE_H__*/
