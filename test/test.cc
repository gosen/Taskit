#define BOOST_TEST_MODULE Taskit
#define BOOST_TEST_DYN_LINK

#include "taskit.hpp"

#include <boost/test/unit_test.hpp>
#include <future>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

class Ctx
{
public:

    void storeInfo(const std::string& s)
    {
        extractedInfo_ = s;
    }

    friend std::ostream& operator<<(std::ostream& os, const Ctx& ctx)
    {
        return os << ctx.extractedInfo_;
    }

private:

    std::string extractedInfo_;
};

struct A
{
    auto operator()(std::ostream& os, Ctx& ctx) const
    {
        ctx.storeInfo("A A A");
        os << " " << ctx << " ...";
        return 'A';
    }
};

struct B
{
    auto operator()(std::ostream& os, Ctx& ctx) const
    {
        ctx.storeInfo("BB BB BB");
        os << " " << ctx << " ...";
        return 'B';
    }
};

struct C
{
    auto operator()(std::ostream& os, Ctx& ctx) const
    {
        ctx.storeInfo("CCC CCC CCC");
        os << " " << ctx << " ...";
        return 'C';
    }
};

auto d(std::ostream& os, Ctx& ctx)
{
    ctx.storeInfo("dddd dddd dddd");
    os << " " << ctx << " ...";
    return 'd';
}

struct P
{
    std::string s_;

public:

    P(int times, const std::string s) : s_( s )
    {
        for(int i = 1; i < times; ++i) s_ += s;
    }

    auto operator()(std::ostream& os, Ctx& ctx) const
    {
        ctx.storeInfo( s_ );
        os << " " << ctx << " ...";
        return 'P';
    }
};

BOOST_AUTO_TEST_CASE( simple_sequence_test )
{
    using namespace std;

    char letter = 'e';
    auto e = [letter](std::ostream& os, Ctx& ctx)
    {
        ctx.storeInfo("eeeee eeeee eeeee");
        os << " " << ctx << " ...";
        return letter;
    };

    A a;

    std::stringstream res;

    cout << "Create TaskSequence..." << endl;
    auto parser = taskit::make_TaskSequence(
                                              taskit::make_TaskType( std::move( a ) ),
                                              taskit::make_TaskType( B{} ),
                                              taskit::make_TaskType<C>(),
                                              taskit::make_TaskType( std::move( d ) ),
                                              taskit::make_TaskType( std::move( e ) ),
                                              taskit::make_TaskType<P>(4, "PpPpPp")
                                             );

    Ctx ctx;

    auto ret = parser(res, ctx);

    std::cout << "'" << ret << "' was inspected: " << ctx << std::endl;

    std::cout << "Parsed info ==> " << res.str() << std::endl;
    BOOST_CHECK( res.str() == " A A A ... BB BB BB ... CCC CCC CCC ... dddd dddd dddd ... eeeee eeeee eeeee ... PpPpPpPpPpPpPpPpPpPpPpPp ..." );
}

BOOST_AUTO_TEST_CASE( simple_selector_test )
{
    using namespace std;

    char letter = 'e';
    auto e = [letter](std::ostream& os, Ctx& ctx)
    {
        ctx.storeInfo("eeeee eeeee eeeee");
        os << " " << ctx << " ...";
        return letter;
    };

    A a;

    constexpr const char stages[] = { 'A', 'B', 'C', 'd', 'e', 'P' };

    std::stringstream res;

    for( auto parser_selector : stages )
    {
        cout << "Create parser " << parser_selector << "..." << endl;
        auto parser = taskit::make_Task( parser_selector,
                                           taskit::make_TaskType<char, 'A'>( std::move( a ) ),
                                           taskit::make_TaskType<char, 'B'>( B{} ),
                                           taskit::make_TaskType<char, 'C', C>(),
                                           taskit::make_TaskType<char, 'd'>( std::move( d ) ),
                                           taskit::make_TaskType<char, 'e'>( std::move( e ) ),
                                           taskit::make_TaskType<char, 'P', P>( 4, "PpPpPp"s )
                                         );

        Ctx ctx;

        auto ret = parser(res, ctx);

        std::cout << "'" << ret << "' was inspected: " << ctx << std::endl;
        BOOST_CHECK( ret == parser_selector );
    }

    std::cout << "Parsed info ==> " << res.str() << std::endl;
    BOOST_CHECK( res.str() == " A A A ... BB BB BB ... CCC CCC CCC ... dddd dddd dddd ... eeeee eeeee eeeee ... PpPpPpPpPpPpPpPpPpPpPpPp ..." );
}

BOOST_AUTO_TEST_CASE( packedTask_test )
{
    char letter = 'e';
    auto e = [letter](std::ostream& os, Ctx& ctx)
    {
        ctx.storeInfo("eeeee eeeee eeeee");
        os << " " << ctx << " ...";
        return letter;
    };

    A a;

    constexpr const char stages[] = { 'A', 'B', 'C', 'd', 'e' };

    std::stringstream res;

    for( auto parser_selector : stages )
    {
        std::cout << "Create parser " << parser_selector << "..." << std::endl;
        auto parser = taskit::make_Task( parser_selector,
                                           taskit::make_TaskType<char, 'A'>( std::move( a ) ),
                                           taskit::make_TaskType<char, 'B'>( B{} ),
                                           taskit::make_TaskType<char, 'C', C>(),
                                           taskit::make_TaskType<char, 'd'>( std::move( d ) ),
                                           taskit::make_TaskType<char, 'e'>( std::move( e ) )
                                         );

        Ctx ctx;
        std::packaged_task<char(std::ostream&, Ctx&)> task( std::bind(parser, std::ref(res), std::ref(ctx)) );
        std::future<char> result = task.get_future();

        task(res, ctx);

        auto ret = result.get();

        std::cout << "'" << ret << "' was inspected: " << ctx << std::endl;
        BOOST_CHECK( ret == parser_selector );
    }

    std::cout << "Parsed info ==> " << res.str() << std::endl;
    BOOST_CHECK( res.str() == " A A A ... BB BB BB ... CCC CCC CCC ... dddd dddd dddd ... eeeee eeeee eeeee ..." );
}

// The objects must have static storage duration and linkage (external pre C++11, internal or
// external in C++11), so that pointers to them can be created at compile time
std::string empty_s;
std::string you_s("you");
std::string the_s("the");
std::string gosen_s("Goʹshen");

BOOST_AUTO_TEST_CASE( dictionary_test )
{
    using namespace std;
    using namespace std::string_literals;

    ifstream in("./test/data.txt");

    using Dict_t = unordered_map<string, vector<string>>;

    Dict_t dict;

    auto wordExtractor = [] (const string& wordToBeExtracted, Dict_t& dict)
    {
        dict[wordToBeExtracted].emplace_back( wordToBeExtracted );
    };

    auto ignore = [] (const string& wordToBeExtracted, Dict_t& dict) {};

    string lookup;
    while( in >> lookup )
    {
        auto word_locator = taskit::make_Task( lookup,
                                                 taskit::make_TaskType<decltype(the_s)  , the_s  >( wordExtractor ),
                                                 taskit::make_TaskType<decltype(you_s)  , you_s  >( wordExtractor ),
                                                 taskit::make_TaskType<decltype(gosen_s), gosen_s>( wordExtractor ),
                                                 taskit::make_TaskType<decltype(empty_s), empty_s>( std::move(ignore) )
                                               );
        word_locator(lookup, dict);
    }

    vector<string> vec_result;
    for( const auto& p : dict )
    {
        for( const auto& s : p.second )
        {
            vec_result.push_back( s );
        }
    }

    sort(vec_result.begin(), vec_result.end());

    stringstream result;
    for( const auto& s : vec_result )
    {
       result << s << " ";
    }
    cout << result.str() << endl;
    BOOST_CHECK( result.str() == "Goʹshen the the the you you ");
}

BOOST_AUTO_TEST_CASE( generate_tasks_before_selected_test )
{
    using namespace std;
    using namespace std::string_literals;

    ifstream in("./test/data.txt");

    using Dict_t = unordered_map<string, vector<string>>;

    Dict_t dict;

    auto wordExtractor = [] (const string& wordToBeExtracted, Dict_t& dict)
    {
        dict[wordToBeExtracted].emplace_back( wordToBeExtracted );
    };

    auto ignore = [] (const string& wordToBeExtracted, Dict_t& dict) {};

    string lookup;
    auto tasks = taskit::make_Tasks<decltype(lookup)>(
                                    taskit::make_TaskType<decltype(the_s)  , the_s  >( wordExtractor ),
                                    taskit::make_TaskType<decltype(you_s)  , you_s  >( wordExtractor ),
                                    taskit::make_TaskType<decltype(gosen_s), gosen_s>( wordExtractor ),
                                    taskit::make_TaskType<decltype(empty_s), empty_s>( std::move(ignore) ));
    while( in >> lookup )
    {
        const auto& word_locator = tasks.select( lookup );
        word_locator(lookup, dict);
    }

    vector<string> vec_result;
    for( const auto& p : dict )
    {
        for( const auto& s : p.second )
        {
            vec_result.push_back( s );
        }
    }

    sort(vec_result.begin(), vec_result.end());

    stringstream result;
    for( const auto& s : vec_result )
    {
       result << s << " ";
    }
    cout << result.str() << endl;
    BOOST_CHECK( result.str() == "Goʹshen the the the you you ");
}

BOOST_AUTO_TEST_CASE( tasks_sizes_test )
{
    char parser_selector = 'A';
    auto parser = taskit::make_Task( parser_selector,
                                     taskit::make_TaskType<char, 'A', A>(),
                                     taskit::make_TaskType<char, 'B', B>(),
                                     taskit::make_TaskType<char, 'C', C>()
                                   );

    BOOST_CHECK( sizeof parser == 1 );
}
