#ifndef __EXPORT_H__
#define __EXPORT_H__

#include <string>
#include <vector>

enum {
    exact, dice, cosine, jaccard, overlap,
};

class writer
{
protected:
    void *m_dbw;
    void *m_gen;

public:
    writer(const char *filename, int n = 3, bool be = false);
    virtual ~writer();
    void insert(const char *string);
    void close();
};

class reader
{
protected:
    void *m_dbr;

public:
    reader(const char *filename);
    virtual ~reader();
    std::vector<std::string> retrieve(const char *query);
    void close();

public:
    int measure;
    double threshold;
};

#endif/*__EXPORT_H__*/

