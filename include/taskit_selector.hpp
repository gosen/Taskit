#ifndef __TASKIT_SELECTOR_H__
#define __TASKIT_SELECTOR_H__

#include "taskit_tasks.hpp"

namespace taskit {

template<typename Head, typename... Tails>
struct ElseTaskSelector : protected ElseTaskSelector<Tails...>, protected FunctionHolder<Head>
{
protected:

    using task_t = typename Head::type;

    constexpr ElseTaskSelector(task_t type, Head head, Tails... tails)
        : ElseTaskSelector<Tails...>(type, tails...)
        , FunctionHolder<Head>( head.getFunctorRef() )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return ElseTaskSelector<Tails...>::type() == Head::value() ?
            this->FunctionHolder<Head>::exe( std::forward<Args>(args)... ) :
            ElseTaskSelector<Tails...>::operator()( std::forward<Args>(args)... );
    }
};

template<typename Tail>
struct ElseTaskSelector<Tail> : protected FunctionHolder<Tail>
{
    using task_t = typename Tail::type;

protected:

    constexpr ElseTaskSelector(task_t type, Tail tail)
        : FunctionHolder<Tail>( tail.getFunctorRef() )
        , type_( type )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return this->FunctionHolder<Tail>::exe( std::forward<Args>(args)... );
    }

    constexpr auto type() const noexcept { return type_; }

    template<typename T>
    void select(T&& type)
    {
        type_ = std::forward<T>( type );
    }

private:

    task_t type_;
};

template<typename... TaskList>
class Task : private ElseTaskSelector<TaskList...>
{
public:

    template<typename T>
    static Task make_Task(T type, TaskList&&... taskList)
    {
        return Task(type, std::forward<TaskList>(taskList)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
       return ElseTaskSelector<TaskList...>::operator()( std::forward<Args>(args)... );
    }

    template<typename T>
    auto& select(T&& type)
    {
        ElseTaskSelector<TaskList...>::select( std::forward<T>( type ) );
        return *this;
    }

private:

    template<typename T>
    constexpr Task(T type, TaskList&&... taskList)
        : ElseTaskSelector<TaskList...>(type, std::forward<TaskList>(taskList)...)
    {}
};

template<typename T, typename... TASKS_LIST>
constexpr auto make_Task(T type, TASKS_LIST&&... args)
{
    return Task<TASKS_LIST...>::make_Task(type, std::forward<TASKS_LIST>(args)...);
}

template<typename T, typename... TASKS_LIST>
constexpr auto make_Tasks(TASKS_LIST&&... args)
{
    return Task<TASKS_LIST...>::make_Task(T(), std::forward<TASKS_LIST>(args)...);
}

} // taskit namespace

#endif // __TASKIT_SELECTOR_H__
