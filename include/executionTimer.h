#ifndef WGPU_PS_EXECUTIONTIMER_H
#define WGPU_PS_EXECUTIONTIMER_H

#include <chrono>
#include <type_traits>
#include <sstream>
#include <iostream>

template <class Resolution = std::chrono::microseconds>
class ExecutionTimer {
public:
    using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
            std::chrono::high_resolution_clock,
            std::chrono::steady_clock>;

private:
    const Clock::time_point mStart = Clock::now();

public:
    ExecutionTimer() = default;
    ~ExecutionTimer() = default;
//    ~ExecutionTimer() {
//        const auto end = Clock::now();
//        std::ostringstream strStream;
//        strStream << "Destructor Elapsed: "
//                  << std::chrono::duration_cast<Resolution>(end - mStart).count()
//                  << std::endl;
//        std::cout << strStream.str() << std::endl;
//    }

    inline void stop(const std::string& text) {
        const auto end = Clock::now();
        std::ostringstream strStream;
        strStream << text
                  << std::chrono::duration_cast<Resolution>(end - mStart).count()
                  << std::endl;
        std::cout << strStream.str() << std::endl;
    }
};

#endif //WGPU_PS_EXECUTIONTIMER_H
