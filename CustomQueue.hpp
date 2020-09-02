#pragma once

#ifndef __CUSTOM_QUEUE_V0__
#define __CUSTOM_QUEUE_V0__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

struct AVPacket;
struct AVFrame;

template <typename T>
class CustomQueue
{
public:

    CustomQueue (const size_t sizeMax);

    bool push(T &&data);
    bool pop(T &data);

    void finished();
    void quit();

private:
    std::queue<T> queue;
    typename std::queue<T>::size_type mSizeMax;

    std::mutex mutex;
    std::condition_variable fullQueue;
    std::condition_variable emptyQueue;

    std::atomic_bool mQuit{false};
    std::atomic_bool mFinished{false};

};

using PacketQueue = CustomQueue<std::unique_ptr<AVPacket, std::function<void(AVPacket*)>>>;
using FrameQueue = CustomQueue<std::unique_ptr<AVFrame, std::function<void(AVFrame*)>>>;

template <typename T>
CustomQueue<T>::CustomQueue(const size_t sizeMax): mSizeMax{sizeMax}
{

}

template <typename T>
bool CustomQueue<T>::push(T &&data)
{
    std::unique_lock<std::mutex> lock(mutex);

    while(!mQuit && !mFinished)
    {
        if(queue.size() < mSizeMax)
        {
            queue.push(std::move(data));
            emptyQueue.notify_all();
            return true;
        }
        else
        {
            fullQueue.wait(lock);
        }
    }

    return false;
}

template <typename T>
bool CustomQueue<T>::pop(T &data)
{
    std::unique_lock<std::mutex> lock(mutex);

    while(!mQuit)
    {
        if(!queue.empty())
        {
            data = std::move(queue.front());
            queue.pop();
            fullQueue.notify_all();

            return true;
        }
        else if(queue.empty() && mFinished)
        {
            return false;
        }
        else
        {
            emptyQueue.wait(lock);
        }
        
    }

    return false;
}

template <typename T>
void CustomQueue<T>::finished()
{
    mFinished = true;
    emptyQueue.notify_all();
}

template <typename T>
void CustomQueue<T>::quit()
{
    mQuit = true;
    emptyQueue.notify_all();
    fullQueue.notify_all();
}

#endif //__CUSTOM_QUEUE_V0__