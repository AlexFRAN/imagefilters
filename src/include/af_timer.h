#include <chrono>
#include <iostream>

namespace af
{
    class Timer
    {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start_point;

    public:
        Timer()
        {
            m_start_point = std::chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            Stop();
        }

        void Stop()
        {
            auto end_timepoint = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start_point).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

            auto duration = end - start;
            double ms = duration * 0.001;

            std::cout << "Duration: " << ms << "ms\n";
        }
    };
};