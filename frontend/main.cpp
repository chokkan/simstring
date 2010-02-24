/*
 *      SimString frontend.
 *
 * Copyright (c) 2009,2010 Naoaki Okazaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the authors nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* $Id$ */

#include <cstdlib>
#include <ctime>
#include <ios>
#include <iostream>
#include <iterator>
#include <locale>
#include <locale.h>
#include <string>
#include <typeinfo>
#include <vector>
#include <simstring/simstring.h>

#include "optparse.h"

class option
{
public:
    enum {
        MODE_RETRIEVE = 0,
        MODE_BUILD,
        MODE_HELP,
        MODE_VERSION,
    };

    enum {
        CC_CHAR = 0,    // char
        CC_WCHAR,       // wchar_t
    };

    enum {
        QT_EXACT = 0,
        QT_DICE,
        QT_COSINE,
        QT_JACCARD,
        QT_OVERLAP,
    };

    int mode;
    int code;
    std::string name;

    int ngram_size;
    bool be;
    int measure;
    double threshold;
    bool echo_back;
    bool quiet;
    bool benchmark;

public:
    option() :
        mode(MODE_RETRIEVE),
        code(CC_CHAR),
        name(""),
        ngram_size(3),
        be(false),
        measure(QT_COSINE),
        threshold(0.7),
        echo_back(false),
        quiet(false),
        benchmark(false)
    {
    }
};

class option_parser :
    public option,
    public optparse
{
    BEGIN_OPTION_MAP_INLINE()
        ON_OPTION(SHORTOPT('b') || LONGOPT("build"))
            mode = MODE_BUILD;

        ON_OPTION_WITH_ARG(SHORTOPT('d') || LONGOPT("database"))
            name = arg;

        ON_OPTION(SHORTOPT('u') || LONGOPT("unicode"))
            code = CC_WCHAR;

        ON_OPTION_WITH_ARG(SHORTOPT('n') || LONGOPT("ngram"))
            ngram_size = std::atoi(arg);

        ON_OPTION(SHORTOPT('m') || LONGOPT("mark"))
            be = true;

        ON_OPTION_WITH_ARG(SHORTOPT('s') || LONGOPT("similarity"))
            if (std::strcmp(arg, "exact") == 0) {
                measure = QT_EXACT;
            } else if (std::strcmp(arg, "dice") == 0) {
                measure = QT_DICE;
            } else if (std::strcmp(arg, "cosine") == 0) {
                measure = QT_COSINE;
            } else if (std::strcmp(arg, "jaccard") == 0) {
                measure = QT_JACCARD;
            } else if (std::strcmp(arg, "overlap") == 0) {
                measure = QT_OVERLAP;
            }

        ON_OPTION_WITH_ARG(SHORTOPT('t') || LONGOPT("threshold"))
            threshold = std::atof(arg);

        ON_OPTION(SHORTOPT('e') || LONGOPT("echo"))
            echo_back = true;

        ON_OPTION(SHORTOPT('q') || LONGOPT("quiet"))
            quiet = true;

        ON_OPTION(SHORTOPT('p') || LONGOPT("benchmark"))
            benchmark = true;

        ON_OPTION(SHORTOPT('v') || LONGOPT("version"))
            mode = MODE_VERSION;

        ON_OPTION(SHORTOPT('h') || LONGOPT("help"))
            mode = MODE_HELP;

    END_OPTION_MAP()
};

int usage(std::ostream& os, const char *argv0)
{
    os << "USAGE: " << argv0 << " [OPTIONS]" << std::endl;
    os << "This utility finds strings in the database (DB) such that they have similarity," << std::endl;
    os << "in the similarity measure (SIM), no smaller than the threshold (TH) with" << std::endl;
    os << "queries read from STDIN. When -b (--build) option is specified, this utility" << std::endl;
    os << "builds a database (DB) for strings read from STDIN." << std::endl;
    os << std::endl;
    os << "OPTIONS:" << std::endl;
    os << "  -b, --build           build a database for strings read from STDIN" << std::endl;
    os << "  -d, --database=DB     specify a database file" << std::endl;
    os << "  -u, --unicode         use Unicode (wchar_t) for representing characters" << std::endl;
    os << "  -n, --ngram=N         specify the unit of n-grams (DEFAULT=3)" << std::endl;
    os << "  -m, --mark            include marks for begins and ends of strings" << std::endl;
    os << "  -s, --similarity=SIM  specify a similarity measure (DEFAULT='cosine'):" << std::endl;
    os << "      exact                 exact match" << std::endl;
    os << "      dice                  dice coefficient" << std::endl;
    os << "      cosine                cosine coefficient" << std::endl;
    os << "      jaccard               jaccard coefficient" << std::endl;
    os << "      overlap               overlap coefficient" << std::endl;
    os << "  -t, --threshold=TH    specify the threshold (DEFAULT=0.7)" << std::endl;
    os << "  -e, --echo-back       echo back query strings to the output" << std::endl;
    os << "  -q, --quiet           suppress supplemental information from the output" << std::endl;
    os << "  -p, --benchmark       show benchmark result (retrieved strings are suppressed)" << std::endl;
    os << "  -v, --version         show this version information and exit" << std::endl;
    os << "  -h, --help            show this help message and exit" << std::endl;
    os << std::endl;
    return 0;
}

int version(std::ostream& os)
{
    os << SIMSTRING_NAME " ";
    os << SIMSTRING_MAJOR_VERSION << "." << SIMSTRING_MINOR_VERSION << " ";
    os << SIMSTRING_COPYRIGHT << std::endl;
    os << std::endl;
    return 0;
}

template <class char_type, class istream_type>
int build(option& opt, istream_type& is)
{
    typedef std::basic_string<char_type> string_type;
    typedef simstring::ngram_generator ngram_generator_type;
    typedef simstring::writer_base<string_type, ngram_generator_type> writer_type;
    
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    // Show the copyright information.
    version(os);

    // Show parameters for database construction.
    os << "Constructing the database" << std::endl;
    os << "Database name: " << opt.name << std::endl;
    os << "N-gram length: " << opt.ngram_size << std::endl;
    os << "Begin/end marks: " << std::boolalpha << opt.be << std::endl;
    os << "Char type: " << typeid(char_type).name() << " (" << sizeof(char_type) << ")" << std::endl;
    os.flush();

    // Open the database for construction.
    clock_t clk = std::clock();
    ngram_generator_type gen(opt.ngram_size, opt.be);
    writer_type db(gen, opt.name);
    if (db.fail()) {
        es << "ERROR: " << db.error() << std::endl;
        return 1;
    }

    // Insert every string from STDIN into the database.
    int n = 0;
    for (;;) {
        // Read a line.
        string_type line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }

        // Insert the string.
        if (!db.insert(line)) {
            es << "ERROR: " << db.error() << std::endl;
            return 1;
        }

        // Progress report.
        if (!opt.quiet && ++n % 10000 == 0) {
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
    os << "Total number of strings: " << n << std::endl;
    os << "Seconds required: "
        << (std::clock() - clk) / (double)CLOCKS_PER_SEC << std::endl;
    os << std::endl;
    os.flush();

    return 0;
}

// widen for strings only with ASCII characters.
template <class char_type>
std::basic_string<char_type> widen(const std::string& str)
{
    std::basic_string<char_type> dst;
    std::string::const_iterator it;
    for (it = str.begin();it != str.end();++it) {
        dst += static_cast<char_type>(*it);
    }
    return dst;
}

template <class char_type, class istream_type, class ostream_type>
int retrieve(option& opt, istream_type& is, ostream_type& os)
{
    typedef std::basic_string<char_type> string_type;
    typedef std::vector<string_type> strings_type;
    typedef simstring::reader_base reader_type;

    std::ostream& es = std::cerr;

    // Open the database.
    reader_type db;
    if (!db.open(opt.name)) {
        es << "ERROR: " << db.error() << std::endl;
        return 1;
    }

    // Check the size of characters.
    if (db.char_size() != sizeof(char_type)) {
        es << "ERROR: Inconsistent character encoding " <<
            "(DB:" << db.char_size() << ", " <<
            "CUR:" << sizeof(char_type) << "): " << std::endl;
        es << "This problem may be solved by specifying -u (--unicode) option." << std::endl;
        return 1;
    }

    int num_queries = 0;
    int num_retrieved = 0;
    clock_t clk_total = 0;
    for (;;) {
        // Read a line.
        string_type line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }

        // Issue a query.
        strings_type xstrs;
        clock_t clk = std::clock();
        switch (opt.measure) {
        case option::QT_EXACT:
            db.retrieve<simstring::measure::exact>(
                line,
                opt.threshold,
                std::back_inserter(xstrs)
                );
            break;
        case option::QT_DICE:
            db.retrieve<simstring::measure::dice>(
                line,
                opt.threshold,
                std::back_inserter(xstrs)
                );
            break;
        case option::QT_COSINE:
            db.retrieve<simstring::measure::cosine>(
                line,
                opt.threshold,
                std::back_inserter(xstrs)
                );
            break;
        case option::QT_JACCARD:
            db.retrieve<simstring::measure::jaccard>(
                line,
                opt.threshold,
                std::back_inserter(xstrs)
                );
            break;
        case option::QT_OVERLAP:
            db.retrieve<simstring::measure::overlap>(
                line,
                opt.threshold,
                std::back_inserter(xstrs)
                );
            break;
        }
        clock_t elapsed = (std::clock() - clk);

        // Update stats.
        clk_total += elapsed;
        num_retrieved += (int)xstrs.size();
        ++num_queries;

        // Do not output results when the benchmarking flag is on.
        if (!opt.benchmark) {
            // Output the query string if necessary.
            if (opt.echo_back) {
                os << line << std::endl;
            }

            // Output the retrieved strings.
            typename strings_type::const_iterator it;
            for (it = xstrs.begin();it != xstrs.end();++it) {
                os << os.widen('\t') << *it << std::endl;
            }
            os.flush();
        }

        // Do not output information when the quiet flag is on.
        if (!opt.quiet) {
            os <<
                xstrs.size() <<
                widen<char_type>(" strings retrieved (") <<
                (std::clock() - clk) / (double)CLOCKS_PER_SEC <<
                widen<char_type>(" sec)") << std::endl;
        }
    }

    // Output the benchmark information if necessary.
    if (opt.benchmark) {
        os <<
            widen<char_type>("Total number of queries: ") <<
            num_queries << std::endl;
        os <<
            widen<char_type>("Seconds per query: ") <<
            clk_total / (double)CLOCKS_PER_SEC / num_queries << std::endl;
        os <<
            widen<char_type>("Number of retrieved strings per query: ") <<
            num_retrieved / (double)num_queries << std::endl;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    // Parse the command-line options.
    option_parser opt;
    try { 
        int arg_used = opt.parse(argv, argc);
    } catch (const optparse::unrecognized_option& e) {
        std::cerr << "ERROR: unrecognized option: " << e.what() << std::endl;
        return 1;
    } catch (const optparse::invalid_value& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    // Change the locale of wcin and wcout if necessary.
    if (opt.code == option::CC_WCHAR) {
        std::ios_base::sync_with_stdio(false);
        std::locale::global(std::locale("")); 
        std::wcout.imbue(std::locale(""));
        std::wcin.imbue(std::locale(""));
    }

    // Branches for the processing mode.
    switch (opt.mode) {
    case option::MODE_HELP:
        return usage(std::cout, argv[0]);
    case option::MODE_VERSION:
        return version(std::cout);
    case option::MODE_BUILD:
        if (opt.code == option::CC_CHAR) {
            return build<char>(opt, std::cin);
        } else if (opt.code == option::CC_WCHAR) {
            return build<wchar_t>(opt, std::wcin);
        }
        break;
    case option::MODE_RETRIEVE:
        if (opt.code == option::CC_CHAR) {
            return retrieve<char>(opt, std::cin, std::cout);
        } else if (opt.code == option::CC_WCHAR) {
            return retrieve<wchar_t>(opt, std::wcin, std::wcout);
        }
        break;
    }

    // An unknown processing mode.
    return 1;
}
