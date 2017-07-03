#ifndef __TASKIT_SEQUENCE_H__
#define __TASKIT_SEQUENCE_H__

#include "taskit_tasks.hpp"

namespace taskit {

template<typename Head, typename... Tails>
struct NextTaskSequence : protected NextTaskSequence<Tails...>, protected FunctionHolder<Head>
{
protected:

    constexpr NextTaskSequence(Head head, Tails... tails)
        : NextTaskSequence<Tails...>( tails... )
        , FunctionHolder<Head>( head.getFunctorRef() )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        this->FunctionHolder<Head>::exe( std::forward<Args>(args)... );
        return NextTaskSequence<Tails...>::operator()( std::forward<Args>(args)... );
    }
};

template<typename Tail>
struct NextTaskSequence<Tail> : protected FunctionHolder<Tail>
{
protected:

    constexpr NextTaskSequence(Tail tail)
        : FunctionHolder<Tail>( tail.getFunctorRef() )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return this->FunctionHolder<Tail>::exe( std::forward<Args>(args)... );
    }
};

template<typename... TaskList>
class TaskSequence : private NextTaskSequence<TaskList...>
{
public:

    static TaskSequence make_TaskSequence(TaskList&&... taskList)
    {
        return TaskSequence(std::forward<TaskList>(taskList)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
       return NextTaskSequence<TaskList...>::operator()( std::forward<Args>(args)... );
    }

private:

    constexpr TaskSequence(TaskList&&... taskList)
        : NextTaskSequence<TaskList...>(std::forward<TaskList>(taskList)...)
    {}
};

template<typename... TASKS_LIST>
constexpr auto make_TaskSequence(TASKS_LIST&&... args)
{
    return TaskSequence<TASKS_LIST...>::make_TaskSequence( std::forward<TASKS_LIST>(args)... );
}

} // taskit namespace

#endif // __TASKIT_SEQUENCE_H__
