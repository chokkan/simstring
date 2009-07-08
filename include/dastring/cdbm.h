#ifndef __CDBM_H__
#define __CDBM_H__

#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdexcept>

namespace cdbm
{

/**
 * Global constants.
 */
enum {
    VERSION = 1,
    NUM_TABLES = 256,
    BYTEORDER_CHECK = 0x62445371,
};




/**
 * C++ port of SuperFastHash function.
 *  This class is a port of the SuperFastHash proposeed by Paul Hsieh.
 *  @author Paul Hsieh
 *  @see    http://www.azillionmonkeys.com/qed/hash.html
 */
class superfasthash :
    public std::binary_function<const void *, size_t, uint32_t>
{
protected:
    inline static uint16_t get16bits(const char *d)
    {
        return *reinterpret_cast<const uint16_t*>(d);
    }

public:
    inline uint32_t operator() (const void *key, size_t size)
    {
        size_t rem;
        uint32_t hash = size, tmp;
        const char *data = reinterpret_cast<const char *>(key);

        if (size == 0 || data == NULL) return 0;

        rem = size & 3;
        size >>= 2;

        // Main loop
        for (;size > 0; size--) {
            hash  += get16bits (data);
            tmp    = (get16bits (data+2) << 11) ^ hash;
            hash   = (hash << 16) ^ tmp;
            data  += 2*sizeof (uint16_t);
            hash  += hash >> 11;
        }

        // Handle end cases
        switch (rem) {
            case 3: hash += get16bits (data);
                    hash ^= hash << 16;
                    hash ^= data[sizeof (uint16_t)] << 18;
                    hash += hash >> 11;
                    break;
            case 2: hash += get16bits (data);
                    hash ^= hash << 11;
                    hash += hash >> 17;
                    break;
            case 1: hash += *data;
                    hash ^= hash << 10;
                    hash += hash >> 1;
        }

        // Force "avalanching" of final 127 bits
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash;
    }
};




/**
 * The header structure of a chunk.
 */
struct chunkheader_t
{
    int8_t      chunkid[4];     /// Chunk identifier, "CDBM".
    uint32_t    version;        /// Flag (reserved).
    uint32_t	size;           /// Chunk size including this header.
    uint32_t	byteorder;      /// Byte-order indicator.
};

struct tableref_t
{
    uint32_t    offset;     /// Offset to a hash table.
    uint32_t    num;        /// Number of elements in the hash table.
};


static uint32_t get_data_begin()
{
    return (sizeof(chunkheader_t) + sizeof(tableref_t) * NUM_TABLES);
}



class write_exception : public std::invalid_argument
{
public:
    write_exception(const std::string& msg)
        : std::invalid_argument(msg)
    {
    }
};

class writer
{
protected:
    struct bucket
    {
	    uint32_t	hash;		/// Hash value of the record.
	    uint32_t	offset;		/// Offset address to the actual record.

        bucket() : hash(0), offset(0) {}
        bucket(uint32_t h, uint32_t o) : hash(h), offset(o) {}
    };

    typedef std::vector<bucket> hashtable;

protected:
    std::ofstream&  m_os;
    uint32_t        m_begin;
    uint32_t        m_cur;
	hashtable       m_ht[NUM_TABLES];	/// Hash tables.

public:
    writer(std::ofstream& os) : m_os(os)
    {
        m_begin = m_os.tellp();
        m_cur = get_data_begin();
        m_os.seekp(m_begin + m_cur);
    }

    virtual ~writer()
    {
        close();
    }

    template <class key_t, class value_t>
    void put(const key_t *key, size_t ksize, const value_t *value, size_t vsize)
    {
        // Write out the current record.
        write_uint32((uint32_t)ksize);
        m_os.write(reinterpret_cast<const char *>(key), ksize);
        write_uint32((uint32_t)vsize);
        m_os.write(reinterpret_cast<const char *>(value), vsize);

	    // Compute the hash value and choose a hash table.
	    uint32_t hv = superfasthash()(static_cast<const void *>(key), ksize);
	    hashtable& ht = m_ht[hv % NUM_TABLES];

        // Store the hash value and offset to the hash table.
        ht.push_back(bucket(hv, m_cur));

        // Increment the current position.
        m_cur += sizeof(uint32_t) + ksize + sizeof(uint32_t) + vsize;
    }

    void close()
    {
        // Check the consistency of the stream offset.
        if (m_begin + m_cur != (uint32_t)m_os.tellp()) {
            throw write_exception("inconsistent stream offset");
        }

	    // Store the hash tables. At this moment, the file pointer refers to
		// the offset succeeding the last key/value pair.
	    for (size_t i = 0;i < NUM_TABLES;++i) {
		    hashtable& ht = m_ht[i];

		    // Do not write an empty hash table.
		    if (!ht.empty()) {
			    // An actual table will have the double size; half elements
                // in the table are kept empty.
			    int n = ht.size() * 2;

			    // Allocate the actual table.
                bucket* dst = new bucket[n];

			    // Put hash elements to the table with the open-address method.
                hashtable::const_iterator it;
                for (it = ht.begin();it != ht.end();++it) {
                    int k = (it->hash >> 8) % n;

				    // Find a vacant element.
				    while (dst[k].offset != 0) {
					    k = (k+1) % n;
				    }

				    // Store the hash element.
				    dst[k].hash = it->hash;
				    dst[k].offset = it->offset;
			    }

			    // Write out the new table.
			    for (int k = 0;k < n;++k) {
				    write_uint32(dst[k].hash);
				    write_uint32(dst[k].offset);
			    }

			    // Free the table.
                delete[] dst;
		    }
	    }

	    // Store the current position.
        uint32_t offset = (uint32_t)m_os.tellp();

        // Construct the chunk header.
	    chunkheader_t header;
        memset(&header, 0, sizeof(header));
        memcpy(header.chunkid, "CDBM", 4);
        header.version = VERSION;
        header.size = offset - m_begin;
        header.byteorder = BYTEORDER_CHECK;

	    // Rewind the stream position to the beginning.
        m_os.seekp(m_begin);
        m_os.write(reinterpret_cast<char *>(&header), sizeof(header));

	    // Write references to hash tables. At this moment, dbw->cur points
		// to the offset succeeding the last key/data pair. 
	    for (size_t i = 0;i < NUM_TABLES;++i) {
		    // Offset to the hash table (or zero for non-existent tables).
            write_uint32(m_ht[i].empty() ? 0 : m_cur);
		    // Bucket size is double to the number of elements.
		    write_uint32(m_ht[i].size() * 2);
		    // Advance the offset counter.
            m_cur += sizeof(uint32_t) * 2 * m_ht[i].size() * 2;
	    }

	    // Seek to the last position.
        m_os.seekp(offset);
    }

protected:
    inline void write_uint32(uint32_t value)
    {
        m_os.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }
};

class reader_exception : public std::invalid_argument
{
public:
    reader_exception(const std::string& msg)
        : std::invalid_argument(msg)
    {
    }
};

class reader
{
protected:
    struct bucket_t
    {
	    uint32_t	hash;		/// Hash value of the record.
	    uint32_t	offset;		/// Offset address to the actual record.
    };


    struct hashtable_t
    {
        uint32_t        num;    /// Number of elements in the table.
	    const bucket_t* buckets;/// Buckets (array of bucket).
    };


protected:
	const uint8_t*  m_buffer;           /// Pointer to the memory block.
	size_t          m_size;             /// Size of the memory block.

	const chunkheader_t*    m_header;   /// Chunk header.
	hashtable_t     m_ht[NUM_TABLES];   /// Hash tables.
    size_t          m_n;

public:
    reader() : m_buffer(NULL), m_size(0)
    {
    }

    reader(const void *buffer, size_t size)
    {
        init(buffer, size);
    }

    virtual ~reader()
    {
    }

    bool empty() const
    {
        return (m_buffer == NULL);
    }

    size_t init(const void *buffer, size_t size)
    {
        const uint8_t *p = reinterpret_cast<const uint8_t*>(buffer);

	    // Make sure that the size of the chunk is larger than the minimum size.
	    if (size < get_data_begin()) {
            throw reader_exception("");
	    }

        // Obtain the chunk header.
        m_header = reinterpret_cast<const chunkheader_t*>(p);
        p += sizeof(chunkheader_t);

	    // Check the chunk identifier.
        if (memcmp(m_header->chunkid, "CDBM", 4) != 0) {
            throw reader_exception("incorrect chunk header");
        }

        // Check the byte-order consistency.
        if (m_header->byteorder != BYTEORDER_CHECK) {
            throw reader_exception("inconsistent byte order");
        }

        // Check the chunk size.
        if (size < m_header->size) {
            throw reader_exception("the buffer is too small");
        }

        // Set memory block and size.
        m_buffer = reinterpret_cast<const uint8_t*>(buffer);
        m_size = size;

        // Set pointers to the hash tables.
        m_n = 0;
        const tableref_t* ref = reinterpret_cast<const tableref_t*>(p);
        for (size_t i = 0;i < NUM_TABLES;++i) {
		    if (ref[i].offset) {
			    // Set the buckets.
                m_ht[i].buckets = reinterpret_cast<const bucket_t*>(m_buffer + ref[i].offset);
                m_ht[i].num = ref[i].num;
		    } else {
			    // An empty hash table.
                m_ht[i].buckets = NULL;
                m_ht[i].num = 0;
		    }

		    // The number of records is the half of the table size.
            m_n += (ref[i].num / 2);
	    }

        return (size_t)m_header->size;
    }

    const void* get(const void *key, size_t ksize, size_t* vsize) const
    {
	    uint32_t hv = superfasthash()(key, ksize);
	    const hashtable_t* ht = &m_ht[hv % NUM_TABLES];

	    if (ht->num && ht->buckets != NULL) {
		    int n = ht->num;
		    int k = (hv >> 8) % n;
		    const bucket_t* p = NULL;

		    while (p = &ht->buckets[k], p->offset) {
			    if (p->hash == hv) {
				    const uint8_t *q = m_buffer + p->offset;
                    if (*reinterpret_cast<const uint32_t*>(q) == ksize &&
                        memcmp(key, q + sizeof(uint32_t), ksize) == 0) {
                        q += sizeof(uint32_t) + ksize;
                        if (vsize != NULL) {
                            *vsize = *reinterpret_cast<const uint32_t*>(q);
                        }
                        return q + sizeof(uint32_t);
                    }
			    }
			    k = (k+1) % n;
		    }
	    }

        if (vsize != NULL) {
            *vsize = 0;
        }
        return NULL;
    }

    size_t size() const
    {
        return m_n;
    }

    const uint8_t* buffer() const
    {
        return m_buffer;
    }
};

};

#endif/*__CDBM_H__*/
