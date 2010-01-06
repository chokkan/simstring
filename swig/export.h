#ifndef __SWIG_H__
#define __SWIG_H__

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
    writer(const char *filename, int n);
    virtual ~writer();
    void insert(const char *string);
    void close();
};

class reader
{
protected:
    void *m_dbr;
    void *m_gen;

public:

public:
    reader(const char *filename, int n);
    virtual ~reader();
    std::vector<std::string> retrieve(const char *query);
    void close();

public:
    int measure;
    double threshold;
};

#endif/*__SWIG_H__*/

