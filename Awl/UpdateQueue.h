#pragma once

#include "Awl/QuickList.h"

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
            std::lock_guard<std::mutex> lock(queueMutex);

            //put them directly to FreeBlocks, but not to RenderingMessages
            //note that using PendingMessages.clear() here will result in memory leak
            FreeBlocks.push_back(PendingMessages);
        }

        //called by UI thread to propagate changes to render thread
        void Push(const std::function<void(Args ...)> & func)
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            Message * p_message = nullptr;

            if (FreeBlocks.empty())
            {
                p_message = new Message(func);
            }
            else
            {
                p_message = FreeBlocks.pop_front();
            }

            p_message->Func = func;

            PendingMessages.push_back(p_message);
        }

        //called by render thread to apply changes queued by UI thread
        void ApplyUpdates(Args ... args)
        {
            PrepareData();

            //we do not lock anything while applying the changes
            //so Push can be called while ApplyUpdates is still executed
            for (typename MessageQueue::const_iterator i = RenderingMessages.begin(); i != RenderingMessages.end(); ++i)
            {
                i->Func(args ...);
            }

            FreeData();
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
        //	return RenderingMessages.empty();
        //}

        void PrepareData()
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            //moove all the messages from PendingMessages to RenderingMessages
            //very short operation that actually performs a few assignments
            RenderingMessages = std::move(PendingMessages);
        }

        void FreeData()
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            for (Message * p_message : RenderingMessages)
            {
                //release lambda expression
                p_message->Func = std::function<void(Args ...)>();
            }

            //recycle RenderingMessages (also very short operation)
            FreeBlocks.push_back(RenderingMessages);
        }

        std::mutex queueMutex;

        //messages (changes) queued by UI thread while rendering operation is in progress
        MessageQueue PendingMessages;

        //we recycle freed queue elements to avoid superfluous dynamic memory allocation
        MessageQueue FreeBlocks;

        //messages (changes) that render thread is applying at the moment
        MessageQueue RenderingMessages;
    };
}
