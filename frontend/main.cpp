#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <simstring/simstring.h>

#include "optparse.h"

typedef std::string string_type;
typedef std::vector<string_type> strings_type;
typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<string_type, ngram_generator_type> writer_type;
typedef simstring::reader_base<string_type> reader_type;
typedef simstring::query::exact<string_type, ngram_generator_type> query_exact_type;
typedef simstring::query::cosine<string_type, ngram_generator_type> query_cosine_type;
typedef simstring::query::dice<string_type, ngram_generator_type> query_dice_type;
typedef simstring::query::jaccard<string_type, ngram_generator_type> query_jaccard_type;
typedef simstring::query::overlap<string_type, ngram_generator_type> query_overlap_type;

class option : public optparse
{
public:
    enum {
        MODE_INTERACTIVE,
        MODE_BUILD,
        MODE_HELP,
    };

    enum {
        QT_EXACT,
        QT_DICE,
        QT_COSINE,
        QT_JACCARD,
        QT_OVERLAP,
    };

    int mode;
    std::string name;

    int ngram_size;
    int query_type;
    double threshold;
    
public:
    option() :
        mode(MODE_INTERACTIVE),
        name(""),
        ngram_size(3),
        query_type(QT_EXACT),
        threshold(1.)
    {
    }

    BEGIN_OPTION_MAP_INLINE()
        ON_OPTION(SHORTOPT('b') || LONGOPT("build"))
            mode = MODE_BUILD;

        ON_OPTION_WITH_ARG(SHORTOPT('d') || LONGOPT("database"))
            name = arg;

        ON_OPTION_WITH_ARG(SHORTOPT('n') || LONGOPT("ngram"))
            ngram_size = std::atoi(arg);

        ON_OPTION(SHORTOPT('h') || LONGOPT("help"))
            mode = MODE_HELP;

    END_OPTION_MAP()
};

int usage(std::ostream& os, const char *argv0)
{
    os << "USAGE: " << argv0 << " [OPTIONS]" << std::endl;
    os << std::endl;
    return 0;
}

int build(option& opt)
{
    int n = 0;
    clock_t clk;
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    // Show parameters for database construction.
    os << "Constructing the database" << std::endl;
    os << "Database name: " << opt.name << std::endl;
    os << "N-gram length: " << opt.ngram_size << std::endl;
    os.flush();

    // Open the database for construction.
    clk = std::clock();
    ngram_generator_type gen(opt.ngram_size);
    writer_type db(gen, opt.name);
    if (db.fail()) {
        es << "ERROR: " << db.error() << std::endl;
        return 1;
    }

    // Insert every string from STDIN into the database.
    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }

        if (!db.insert(line)) {
            es << "ERROR: " << db.error() << std::endl;
            return 1;
        }

        // Progress report.
        if (++n % 10000 == 0) {
            os << "Number of strings: " << n << std::endl;
            os.flush();
        }
    }
    os << "Number of strings: " << n << std::endl;
    os << std::endl;
    os.flush();

    // Finalize the database.
    os << "Flushing the database" << std::endl;
    if (!db.close()) {
        es << "ERROR: " << db.error() << std::endl;
        return 1;
    }
    os << std::endl;

    // Report the elaped time for construction.
    os << "Seconds required: "
        << (std::clock() - clk) / (double)CLOCKS_PER_SEC << std::endl;
    os << std::endl;
    os.flush();

    return 0;
}

int interactive(option& opt)
{
    reader_type db;
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    db.open(opt.name);

    ngram_generator_type gen(opt.ngram_size);

    for (;;) {
        // Show a prompt.
        switch (opt.query_type) {
        case option::QT_EXACT:
            os << "[exact]> ";
            break;
        case option::QT_DICE:
            os << "[dice>=" << opt.threshold << "]> ";
            break;
        case option::QT_COSINE:
            os << "[cosine>=" << opt.threshold << "]> ";
            break;
        case option::QT_JACCARD:
            os << "[jaccard>=" << opt.threshold << "]> ";
            break;
        case option::QT_OVERLAP:
            os << "[overlap>=" << opt.threshold << "]> ";
            break;
        }

        // Read a line.
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            os << std::endl;
            break;
        }

        // Check if the line indicates a command.
        if (line.compare(0, 16, ":set query exact") == 0) {
            opt.query_type = option::QT_EXACT;

        } else if (line.compare(0, 16, ":set query dice ") == 0) {
            opt.query_type = option::QT_DICE;
            opt.threshold = std::atof(line.c_str() + 16);

        } else if (line.compare(0, 18, ":set query cosine ") == 0) {
            opt.query_type = option::QT_COSINE;
            opt.threshold = std::atof(line.c_str() + 18);

        } else if (line.compare(0, 19, ":set query jaccard ") == 0) {
            opt.query_type = option::QT_JACCARD;
            opt.threshold = std::atof(line.c_str() + 19);

        } else if (line.compare(0, 19, ":set query overlap ") == 0) {
            opt.query_type = option::QT_OVERLAP;
            opt.threshold = std::atof(line.c_str() + 19);

        } else if (line == ":help") {
            //usage_interactive(os);

        } else if (line == ":quit") {
            break;

        } else {
            // The line is a query.
            strings_type xstrs;
            clock_t clk = std::clock();

            // Issue a query.
            switch (opt.query_type) {
            case option::QT_EXACT:
                db.retrieve(
                    query_exact_type(gen, line),
                    std::back_inserter(xstrs)
                    );
                break;
            case option::QT_DICE:
                db.retrieve(
                    query_dice_type(gen, line, opt.threshold),
                    std::back_inserter(xstrs)
                    );
                break;
            case option::QT_COSINE:
                db.retrieve(
                    query_cosine_type(gen, line, opt.threshold),
                    std::back_inserter(xstrs)
                    );
                break;
            case option::QT_JACCARD:
                db.retrieve(
                    query_jaccard_type(gen, line, opt.threshold),
                    std::back_inserter(xstrs)
                    );
                break;
            case option::QT_OVERLAP:
                db.retrieve(
                    query_overlap_type(gen, line, opt.threshold),
                    std::back_inserter(xstrs)
                    );
                break;
            }

            // Output the retrieved strings.
            strings_type::const_iterator it;
            for (it = xstrs.begin();it != xstrs.end();++it) {
                os << '\t' << *it << std::endl;
            }
            os << xstrs.size() << " strings retrieved (" <<
                (std::clock() - clk) / (double)CLOCKS_PER_SEC <<
                " sec)" << std::endl;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    option opt;
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    // Show the copyright information.
    os << SIMSTRING_NAME " ";
    os << SIMSTRING_MAJOR_VERSION << "." << SIMSTRING_MINOR_VERSION << " ";
    os << SIMSTRING_COPYRIGHT << std::endl;
    os << std::endl;

    // Parse the command-line options.
    try { 
        int arg_used = opt.parse(argv, argc);
    } catch (const optparse::unrecognized_option& e) {
        es << "ERROR: unrecognized option: " << e.what() << std::endl;
        return 1;
    } catch (const optparse::invalid_value& e) {
        es << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    // Branches for the processing mode.
    switch (opt.mode) {
    case option::MODE_HELP:
        return usage(os, argv[0]);
    case option::MODE_BUILD:
        return build(opt);
    case option::MODE_INTERACTIVE:
        return interactive(opt);
    default:
        return 1;
    }
}
