#include <iostream>
#include <string>
#include <simstring/simstring.h>

void retrieve(
    simstring::reader& dbr,
    const std::string& query,
    int measure,
    double threshold
    )
{
    // Retrieve similar strings into a string vector.
    std::vector<std::string> xstrs;
    dbr.retrieve(query, measure, threshold, std::back_inserter(xstrs));

    // Output the retrieved strings separated by ", ".
    for (int i = 0;i < (int)xstrs.size();++i) {
        std::cout << (i != 0 ? ", " : "") << xstrs[i];
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    // Create a SimString database with two person names.
    simstring::ngram_generator gen(3, false);
    simstring::writer_base<std::string> dbw(gen, "sample.db");

    dbw.insert("Barack Hussein Obama II");
    dbw.insert("James Gordon Brown");
    dbw.close();

    // Open the database for reading.
    simstring::reader dbr;
    
    dbr.open("sample.db");
    retrieve(dbr, "Barack Obama", simstring::cosine, 0.6);
    retrieve(dbr, "Gordon Brown", simstring::cosine, 0.6);
    retrieve(dbr, "Obama", simstring::cosine, 0.6);
    retrieve(dbr, "Obama", simstring::overlap, 1.0);

    return 0;
}
