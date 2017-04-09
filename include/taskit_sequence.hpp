#ifndef __TASKIT_SEQUENCE_H__
#define __TASKIT_SEQUENCE_H__

#include "taskit_tasks.hpp"

namespace taskit {

template<typename Head, typename... Tails>
struct CRTPSequence : protected CRTPSequence<Tails...>, protected FunctionHolder<Head>
{
protected:

    constexpr CRTPSequence(Head head, Tails... tails)
        : CRTPSequence<Tails...>( tails... )
        , FunctionHolder<Head>( head.getFunctorRef() )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        this->FunctionHolder<Head>::exe( std::forward<Args>(args)... );
        return CRTPSequence<Tails...>::operator()( std::forward<Args>(args)... );
    }
};

template<typename Tail>
struct CRTPSequence<Tail> : protected FunctionHolder<Tail>
{
protected:

    constexpr CRTPSequence(Tail tail)
        : FunctionHolder<Tail>( tail.getFunctorRef() )
    {}

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return this->FunctionHolder<Tail>::exe( std::forward<Args>(args)... );
    }
};

template<typename... TaskList>
class TaskSequence : private CRTPSequence<TaskList...>
{
public:

    static TaskSequence make_TaskSequence(TaskList&&... taskList)
    {
        return TaskSequence(std::forward<TaskList>(taskList)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
       return CRTPSequence<TaskList...>::operator()( std::forward<Args>(args)... );
    }

private:

    constexpr TaskSequence(TaskList&&... taskList)
        : CRTPSequence<TaskList...>(std::forward<TaskList>(taskList)...)
    {}
};

template<typename... TASKS_LIST>
constexpr auto make_TaskSequence(TASKS_LIST&&... args)
{
    return TaskSequence<TASKS_LIST...>::make_TaskSequence( std::forward<TASKS_LIST>(args)... );
}

} // taskit namespace

#endif // __TASKIT_SEQUENCE_H__
