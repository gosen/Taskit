#include <iostream>
#include <string>

#include "taskit.hpp"

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
  auto operator()(Ctx& ctx) const
  {
    ctx.storeInfo("A A A");
    return 'A';
  }
};

struct B
{
  auto operator()(Ctx& ctx) const
  {
    ctx.storeInfo("BB BB BB");
    return 'B';
  }
};

struct C
{
  auto operator()(Ctx& ctx) const
  {
    ctx.storeInfo("CCC CCC CCC");
    return 'C';
  }
};

auto d(Ctx& ctx)
{
    ctx.storeInfo("dddd dddd dddd");
    return 'd';
}

#define PASS_OBJECTS

int main(int argc, char* argv[])
{
    using namespace std;

    cout << "Create parser..." << endl;

    auto parser_selector = argc > 1 ? *argv[1] : ' ';

#ifdef PASS_OBJECTS
    char letter = 'e';
    auto e = [letter](Ctx& ctx)
    {
        ctx.storeInfo("eeeee eeeee eeeee");
        return letter;
    };

    C c;
#endif

    auto parser = taskit::make_Task( parser_selector,
#ifdef PASS_OBJECTS
    #if __cplusplus >= 201703L
                                       taskit::make_TaskType<'A', A>(),
                                       taskit::make_TaskType<'B'>( B{} ),
                                       taskit::make_TaskType<'C'>( std::move( c ) ),
                                       taskit::make_TaskType<'d'>( std::move( d ) ),
                                       taskit::make_TaskType<'e'>( std::move( e ) )
    #else
                                       taskit::make_TaskType<char, 'A', A>(),
                                       taskit::make_TaskType<char, 'B'>( B{} ),
                                       taskit::make_TaskType<char, 'C'>( std::move( c ) ),
                                       taskit::make_TaskType<char, 'd'>( std::move( d ) ),
                                       taskit::make_TaskType<char, 'e'>( std::move( e ) )
    #endif
#else
                                       taskit::make_TaskType<char, 'A', A>(),
                                       taskit::make_TaskType<char, 'B', B>(),
                                       taskit::make_TaskType<char, 'C', C>()
#endif
                                     );

    Ctx ctx;

    auto ret = parser( ctx );

    std::cout << "'" << ret << "' was inspected: " << ctx << std::endl;

    std::cout << "C++ standard VERSION: " << __cplusplus << std::endl;
    return 0;
}
