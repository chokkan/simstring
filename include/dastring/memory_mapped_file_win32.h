#ifndef __MEMORY_MAPPED_FILE_WIN32_H__
#define __MEMORY_MAPPED_FILE_WIN32_H__

#include <memory.h>
#include <windows.h>

class memory_mapped_file_win32 :
    public memory_mapped_file_base
{
public:
    typedef size_t size_type;

protected:
	HANDLE	                m_hFile;
	HANDLE	                m_hMapping;
    std::ios_base::openmode m_mode;
	char*	                m_data;
	size_type               m_size;

public:
    memory_mapped_file_win32()
    {
        m_hFile = INVALID_HANDLE_VALUE;
        m_hMapping = INVALID_HANDLE_VALUE;
        m_mode = 0;
        m_data = NULL;
        m_size = 0;
    }

    virtual ~memory_mapped_file_win32()
    {
        close();
    }

    void open(const std::string& path, std::ios_base::openmode mode)
    {
		DWORD dwDesiredAccess = 0;
		DWORD dwCreationDisposition = 0;

        if (mode & std::ios_base::in) {
			dwDesiredAccess |= GENERIC_READ;
			dwCreationDisposition = OPEN_EXISTING;
		}
        if (mode & std::ios_base::out) {
			dwDesiredAccess |= GENERIC_WRITE;
			dwCreationDisposition = CREATE_NEW;
		}
		if (mode & std::ios_base::trunc) {
			dwDesiredAccess = (GENERIC_READ | GENERIC_WRITE);
			dwCreationDisposition = CREATE_ALWAYS;
		}

		m_hFile = CreateFileA(
			path.c_str(),
			dwDesiredAccess,
			0,
			NULL,
			dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (m_hFile != INVALID_HANDLE_VALUE) {
            m_mode = mode;
            this->resize((size_type)GetFileSize(m_hFile, NULL));
        }
	}

    bool is_open() const
    {
        return (m_hFile != INVALID_HANDLE_VALUE);
    }

    void close()
    {
        this->free();
        if (m_hFile != INVALID_HANDLE_VALUE) {
		    CloseHandle(m_hFile);
		    m_hFile = INVALID_HANDLE_VALUE;
        }
	}

    bool resize(size_type size)
    {
	    if (size == 0) {
            this->free();
		    return true;
	    }

        if (m_hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        this->free();
        DWORD flProtect = (m_mode & std::ios_base::out) ? PAGE_READWRITE : PAGE_READONLY;
        m_hMapping = CreateFileMappingA(
		    m_hFile,
		    NULL,
            flProtect,
		    0,
		    (DWORD)size,
		    NULL
		    );

	    if (m_hMapping == NULL) {
		    CloseHandle(m_hFile);
		    m_hFile = NULL;
            return false;
	    }

        DWORD dwDesiredAccess = (m_mode & std::ios_base::out) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
        m_data = (char*)MapViewOfFile(
		    m_hMapping,
		    dwDesiredAccess,
		    0,
		    0,
		    0
		    );

	    if (m_data == NULL) {
		    CloseHandle(m_hMapping);
		    m_hMapping = NULL;
		    CloseHandle(m_hFile);
		    m_hFile = NULL;
            return false;
	    }

	    m_size = size;
        return true;
    }

    void free()
    {
	    if (m_data != NULL) {
		    UnmapViewOfFile(m_data);
		    m_data = NULL;
	    }
	    if (m_hMapping != INVALID_HANDLE_VALUE) {
		    CloseHandle(m_hMapping);
		    m_hMapping = NULL;
	    }
	    m_size = 0;
    }

    size_type size() const
    {
        return m_size;
    }

    char* data() const
    {
        return m_data;
    }

    const char* const_data() const
    {
        return m_data;
    }
   
    static int alignment()
    {
        return 0;
    }
};

#endif/*__MEMORY_MAPPED_FILE_WIN32_H__*/