#ifndef __TASKIT_TASKS_H__
#define __TASKIT_TASKS_H__

#include <utility>
#include <type_traits>

namespace taskit {

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val>
struct PrintType;

enum class ExternalFunctorObjectToBeCached { yes, no };

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val, ExternalFunctorObjectToBeCached external, class Func>
class TaskType
{
    Func f_;

public:

    using type = T;
    using func = Func;
    constexpr static const auto value() noexcept { return val; }

    explicit constexpr TaskType(Func&& f) : f_( std::forward<Func>( f ) ) {}

    static constexpr const auto cacheExternalFunctorObject() noexcept { return ExternalFunctorObjectToBeCached::yes; }
    constexpr Func getFunctorRef() const noexcept( noexcept( f_ ) ) { return f_; }
};

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val, class Func>
struct TaskType<T, val, ExternalFunctorObjectToBeCached::no, Func>
{
    using type = T;
    using func = Func;
    constexpr static const auto value() noexcept { return val; }

    static constexpr const auto cacheExternalFunctorObject() noexcept { return ExternalFunctorObjectToBeCached::no; }
    constexpr Func getFunctorRef() const noexcept( noexcept( Func() ) ) { return Func(); }
};

template<class Holder, ExternalFunctorObjectToBeCached val = Holder::cacheExternalFunctorObject()>
class FunctionHolder
{
    using functor_t = typename Holder::func;

    functor_t f_;

protected:

    explicit constexpr FunctionHolder(functor_t&& func)
        : f_( std::forward<functor_t>( func ) )
    {}

    template<typename... Args>
    constexpr auto exe(Args&&... args) const noexcept( noexcept( FunctionHolder::f_( std::forward<Args>(args)... ) ) )
    {
        return f_( std::forward<Args>(args)... );
    }
};

template<class Holder>
class FunctionHolder<Holder, ExternalFunctorObjectToBeCached::no>
{
    using functor_t = typename Holder::func;

protected:

    explicit constexpr FunctionHolder(functor_t&&) {}

    template<typename... Args>
    constexpr auto exe(Args&&... args) const noexcept( noexcept( functor_t()( std::forward<Args>(args)... ) ) )
    {
        return functor_t()( std::forward<Args>(args)... );
    }
};

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val, class Func>
constexpr auto make_TaskType()
{
    return TaskType<T, val, ExternalFunctorObjectToBeCached::no, Func>();
}

template<class Func>
constexpr auto make_TaskType()
{
    return TaskType<bool, true, ExternalFunctorObjectToBeCached::no, Func>();
}

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val, class Func>
constexpr auto make_TaskType(Func&& func)
{
	return TaskType<T, val, ExternalFunctorObjectToBeCached::yes, Func>( std::forward<Func>(func) );
}

template<class Func>
constexpr auto make_TaskType(Func&& func)
{
	return TaskType<bool, true, ExternalFunctorObjectToBeCached::yes, Func>( std::forward<Func>(func) );
}

template<class Func, typename... Args>
constexpr auto make_TaskType(Args&&... args)
{
    return make_TaskType<Func>( Func( std::forward<Args>(args)... ) );
}

template<typename T, std::conditional_t<std::is_class<T>::value, T&, T> val, class Func, typename... Args>
constexpr auto make_TaskType(Args&&... args)
{
    return make_TaskType<T, val, Func>( Func( std::forward<Args>(args)... ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 201703L

template<auto val, class Func>
constexpr auto make_TaskType()
{
    return make_TaskType<decltype(val), val, Func>();
}

template<auto val, class Func>
constexpr auto make_TaskType(Func&& func)
{
	return make_TaskType<decltype(val), val, Func>( std::forward<Func>(func) );
}

template<auto val, class Func, typename... Args>
constexpr auto make_TaskType(Args&&... args)
{
    return make_TaskType<decltype(val), val, Func>( Func( std::forward<Args>(args)... ) );
}

#endif

} // taskit namespace

#endif // __TASKIT_TASKS_H__
