Taskit
=====

Settle your kit of tasks and let's starts ;-)

Motivation
----------

Maintainability. Scalability. Those are convenient terms to keep in mind while you are developing a complex system. However, they are to be forgotten as soon as you open the door of non-encapsulated designs. I have found such _sort of designs_ so many times in production software that it seems a war that it cannot be win... alone, I meant.

Anyhow, as long as you got experience, you also realize that there are other as you and they are willing to beat some battles. So they get ready by developing the latest powerful "weapons". But now, You must work out with them, so that you can get advantage of them.

### Divide and conquer 

One of the things that most naturally leads to both maintainable and scalable system is _decouple_. If your designs enforce such a strategy, they will evolve _naturally_ with no much pain whenever new features are to be added.  

It should be easier to follow this approach by having a way to do it descriptively:

``` cpp
void process(const std::string& data, std::string& normalizeData)
{
    auto normalizer = make_taskSequence<CollapseTabsIntoSimpleSpaces, ConvertToLowCase, ScapeSymbols>();
    normalizer(data, normalizeData);
}
```
The point is that by splitting your tasks in a well defined way, you will, sooner than later, realize that code reusing is pretty obvious as source code understandability is improved. Soyour test cases do, which leads to a better tested software at the end.

To get all these things just depend on your design decisions. Believe or not, they have nothing to do with your WoW organization and how many check-point meetings you attend.

### Target

You already has a lot of instruments to address your designs. This just has been the result of trying to improve a particular problem; protocol parsing. As I have explained above, complex systems live time depend on how well-structured is your architecture and well-adapted to future changes. To fulfill that, the use of object factories used to be a good approach.

``` cpp
std::unique_ptr<Parser> make_parser(MessageType type)
{
    switch( type ) {
        case MSG_A: return make_unique<A_MessageParser>();
        case MSG_B: return make_unique<B_MessageParser>();
        case MSG_C: return make_unique<C_MessageParser>();
        default: return nullptr;
    }
}

void processMessage(RawMessage msg, Context& ctx)
{
    auto parser_ptr = make_parser( findOutType( msg ) );
    parser_ptr->inspect(msg, ctx);   
}
```
Although perfectly fine from maintainability perspective, it might not be so convenient from performance perspective in some cases as dynamic type resolution is required at runtime (__Note__: _it is strongly recomended that you make some measurements so that you can assure there is a bottle neck or unacceptable degradation due to virtual function calls_).

In cases where there are indeed an good reason to avoid virtual function calls, there is another approach to keep such _polymorphic behavior_ without performance penalty.

And that is basically the idea behave _Taskit_.  

Design
--------------

Imagine we have to parser a data packet having this format:

```
------------------------------------------------
 Message Type : Length in bytes : Format
------------------------------------------------
 'A'          : 6               : "A A A"
 'B'          : 8               : "BB BB BB"
 'C'          : 11              : "CCC CCC CCC"
------------------------------------------------
```
Our first approach may be:

``` cpp
class RawMessage;
class Context;
using RawMessage = std::vector<uint8_t>;
using MessageType = char;


class Parser
{
public:
    virtual ~Parser() {}
    virtual int inspect(RawMessage msg, Context& ctx) const
    {
        // Do nothing
        return 0;
    }
};

class A : public Parser
{
public:
    int inspect(RawMessage msg, Context& ctx) const override
    {
        // Here goes the code to decode A format...
        return 0;
    }
};

class B : public Parser
{
public:
    int inspect(RawMessage msg, Context& ctx) const override
    {
        // Here goes the code to decode B format...
        return 0;
    }
};

class C : public Parser
{
public:
    int inspect(RawMessage msg, Context& ctx) const override
    {
        // Here goes the code to decode C format...
        return 0;
    }
};

MessageType findOutType(RawMessage msg)
{
    const char typeUnknown = '0';
    return msg.empty() ? typeUnknown : static_cast<MessageType>( msg[0] );
}

std::unique_ptr<Parser> make_parser(MessageType type)
{
    switch( type ) {
        case 'A': return make_unique<A>();
        case 'B': return make_unique<B>();
        case 'C': return make_unique<C>();
        default: return nullptr;
    }
}

void processMessage(RawMessage msg, Context& ctx)
{
    int ret = 0;
    auto parser_ptr = make_parser( findOutType( msg ) );
    if( parser_ptr ) ret = parser_ptr->inspect(msg, ctx);   
    return ret;
}
```
Even when classes A, B and C are empty, their size is one byte by definition. So, *make_unique* will have to request such memory to the heap _each time a new packet arrives_.

When can skip this problem easily by using preallocated instances:


``` cpp
Parser* make_parser(MessageType type)
{
    static std::vector<Parser> parsers = { A{}, B{}, C{} };

    switch( type ) {
        case 'A': return &parsers[0];
        case 'B': return &parsers[1];
        case 'C': return &parsers[2];
        default: return nullptr;
    }
}
```
Unfortunatelly, that only avoid heap requests but no virtual calls (In fact, another considerations might arise, but let them out of this discussion to keep focus in the main problem). 

But, we have to consider how straightforward is to parser new future formats:

``` cpp
class D : public Parser
{
public:
    int inspect(RawMessage msg, Context& ctx) const override
    {
        // Here goes the code to decode D format...
        return 0;
    }
};

class E : public Parser
{
public:
    int inspect(RawMessage msg, Context& ctx) const override
    {
        // Here goes the code to decode E format...
        return 0;
    }
};

Parser* make_parser(MessageType type)
{
    static std::vector<Parser> parsers = { A{}, B{}, C{}, D{}, E{} };

    switch( type ) {
        case 'A': return &parsers[0];
        case 'B': return &parsers[1];
        case 'C': return &parsers[2];
        case 'C': return &parsers[3];
        case 'C': return &parsers[4];
        default: return nullptr;
    }
}
```
How can we overcome the virtual call problem without missing out our convenient design structure? The answer is generic programing a variadit templates.

We can generate an _if-else_ structure at compile time to be checked at runtime.

``` cpp
class A
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Here goes the code to decode A format...
        return 0;
    }
};

class B
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Here goes the code to decode B format...
        return 0;
    }
};

class C
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Here goes the code to decode C format...
        return 0;
    }
};

class D
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Here goes the code to decode D format...
        return 0;
    }
};

class E
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Here goes the code to decode E format...
        return 0;
    }
};

class Default
{
public:
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Do nothing
        return 0;
    }
};

Parser make_parser(MessageType type)
{
    using namespace taskit;
    return make_Task( type,
                      make_TaskType<char, 'A', A>(),
                      make_TaskType<char, 'B', B>(),
                      make_TaskType<char, 'C', C>(),
                      make_TaskType<char, 'D', D>(),
                      make_TaskType<char, 'E', E>(),
                      make_TaskType<char, '0', Default>());
}

void processMessage(RawMessage msg, Context& ctx)
{
    auto parser = make_parser( findOutType( msg ) );
    return parser(msg, ctx);   
}
```
And that's the idea ;-)

Requirements
------------

This library relies heavily on function return type deduction. Any C++14 compliant compiler is fine.

You also need boost_unit_test_framework library to run UT.

Usage
-----

You can define task...

``` cpp
Parser make_parser(MessageType type)
{
    using namespace taskit;
    return make_Task( type,
                      make_TaskType<char, 'A', A>(),
                      make_TaskType<char, 'B', B>(),
                      make_TaskType<char, 'C', C>(),
                      make_TaskType<char, 'D', D>(),
                      make_TaskType<char, 'E', E>(),
                      make_TaskType<char, '0', Default>());
}
```
... and task sequences:

``` cpp
void process(const std::string& data, std::string& normalizeData)
{
    using namespace taskit;
    auto normalizer = make_TaskSequence<make_TaskType<CollapseTabsIntoSimpleSpaces>(),
                                        make_TaskType<ConvertToLowCase>(),
                                        make_TaskType<ScapeSymbols>());
    normalizer(data, normalizeData);
}
```

Functors, functions and lambdas are supported:

``` cpp
int d(RawMessage msg, Context& ctx)
{
    // Here goes the code to decode D format...
    return 0;
}

class Default
{
    int code_;
public:
    Default(int code) : code_{ code } {}
    int operator()(RawMessage msg, Context& ctx) const
    {
        // Do nothing
        return code_;
    }
};

Parser make_parser(MessageType type)
{
    auto e = [](RawMessage msg, Context& ctx)
    {
        // Here goes the code to decode D format...
        return 0;
    }

    A a;

    using namespace taskit;
    return make_Task( type,
                      make_TaskType<char, 'A'>( std::move( a ) ),
                      make_TaskType<char, 'B'>( B{} ),
                      make_TaskType<char, 'C', C>(),
                      make_TaskType<char, 'D'>( std::move( d ) ),
                      make_TaskType<char, 'E'>( std::move( e ) ),
                      make_TaskType<char, '0', Default>( 100 ));
}
```

As tasks are actually functors, they can be used into packed_task object too:


``` cpp
    auto parser = taskit::make_Task( parser_selector,
                                     taskit::make_TaskType<char, 'A'>( std::move( a ) ),
                                     taskit::make_TaskType<char, 'B'>( B{} ),
                                     taskit::make_TaskType<char, 'C', C>(),
                                     taskit::make_TaskType<char, 'D'>( std::move( d ) ),
                                     taskit::make_TaskType<char, 'E'>( std::move( e ) )
                                   );

    Ctx ctx;
    std::packaged_task<char(std::ostream&, Ctx&)> task( std::bind(parser, std::ref(res), std::ref(ctx)) );
    std::future<char> result = task.get_future();

    task(res, ctx);

    auto ret = result.get();
```

Hope you can see the advantages of this approach :-)
