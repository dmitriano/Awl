#pragma once

#include "Awl/QuickList.h"
#include "Awl/ScopeGuard.h"

#include <mutex>
#include <functional>

namespace awl
{
    template<typename ... Args>
    class UpdateQueue
    {
        struct Message : public awl::quick_link
        {
            Message(const std::function<void(Args ...)> & func) : Func(func)
            {
            }

            std::function<void(Args ...)> Func;
        };

        class MessageQueue : public awl::quick_list<Message>
        {
        public:

            //it is a dependent name, so we need a typename here to specify that it refers to a type
            //typedef typename awl::quick_list<Message>::iterator iterator;

            MessageQueue& operator=(MessageQueue&& other)
            {
                awl::quick_list<Message>::operator=(std::move(other));
                return *this;
            }

            ~MessageQueue()
            {
                typename MessageQueue::iterator i = awl::quick_list<Message>::begin();

                while (i != awl::quick_list<Message>::end())
                {
                    delete *(i++);
                }
            }
        };

    public:

        void ClearPending()
        {
            std::lock_guard<std::recursive_mutex> lock(queueMutex);

            //put them directly to freeBlocks, but not to renderingMessages
            //note that using pendingMessages.clear() here will result in memory leak
            freeBlocks.push_back(pendingMessages);
        }

        //called by UI thread to propagate changes to render thread
        void Push(const std::function<void(Args ...)> & func)
        {
            std::lock_guard<std::recursive_mutex> lock(queueMutex);

            Message * p_message = nullptr;

            if (freeBlocks.empty())
            {
                p_message = new Message(func);
            }
            else
            {
                p_message = freeBlocks.pop_front();
            }

            p_message->Func = func;

            pendingMessages.push_back(p_message);
        }

        //called by render thread to apply changes queued by UI thread
        void ApplyUpdates(Args ... args)
        {
            //if an exception is thrown, the queue stays in a valid state, but remaining updates are lost
            auto guard = awl::make_scope_guard( [this]() { PrepareData();}, [this]() { FreeData();});

            //we do not lock anything while applying the changes
            //so Push can be called while ApplyUpdates is still executed
            for (const Message * p_m : renderingMessages)
            {
                p_m->Func(args ...);
            }
        }

    private:

        //The following commented code does not make a sence because if theoretically
        //there are two clicks in the queue, the first click can create phantoms
        //and the second can select a ball that will explode. So this sutuation 
        //should be handled in GameField.Click(...) function for each certain 
        //case/event, but not for the entire queue.

        //updateQueue.PrepareData();
        //if (!updateQueue.IsEmpty())
        //{
        //	//Field is unable to handle the user input while some action is in progress
        //	//so we stop all the actions before processing the updates
        //	pField->CompletePhantoms();
        //	updateQueue.ApplyUpdates(*pSceneRenderer.get());
        //}
        //updateQueue.FreeData();

        //should be called on render thread after PrepareData()
        //bool IsEmpty()
        //{
        //	return renderingMessages.empty();
        //}

        void PrepareData()
        {
            std::lock_guard<std::recursive_mutex> lock(queueMutex);

            //moove all the messages from pendingMessages to renderingMessages
            //very short operation that actually performs a few assignments
            renderingMessages = std::move(pendingMessages);
        }

        void FreeData()
        {
            std::lock_guard<std::recursive_mutex> lock(queueMutex);

            for (Message * p_message : renderingMessages)
            {
                //release lambda expression
                p_message->Func = std::function<void(Args ...)>();
            }

            //recycle renderingMessages (also very short operation)
            freeBlocks.push_back(renderingMessages);
        }

        //an update can queue another updates by calling Push() while it is processed, so the mutex is recursive
        std::recursive_mutex queueMutex;

        //messages (changes) queued by UI thread while rendering operation is in progress
        MessageQueue pendingMessages;

        //we recycle freed queue elements to avoid superfluous dynamic memory allocation
        MessageQueue freeBlocks;

        //messages (changes) that render thread is applying at the moment
        MessageQueue renderingMessages;
    };
}
