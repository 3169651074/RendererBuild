#include <Global.hpp>

using namespace renderer;

namespace renderer {
    void (*func)() = nullptr;
}

namespace {
    void checkInitError(const char * taskName, TaskOnError task, bool isError) { //枚举类型为int，传值
        if (func == nullptr) {
            SDL_Log("Release resources function is not set!");
        }

        if (isError) {
            switch (task) {
                case PRINT_MASSAGE:
                    SDL_Log("%s Failed: %s\n", taskName, SDL_GetError());
                    break;
                case EXIT_PROGRAM:
                    SDL_Log("%s Failed, Exit Program: %s\n", taskName, SDL_GetError());
                    if (func != nullptr) {
                        func();
                    }
                    exit(EXIT_FAILURE);
                case IGNORE:
                    break;
                default:
                    SDL_Log("Unknown \"task if error\" code: %d\n", task);
            }
        } else {
            SDL_Log("%s...OK\n", taskName);
        }
    }
}

namespace renderer {
    //检查SDL函数返回值
    void sdlCheckErrorInt(int retVal, const char * taskName, TaskOnError errorTask) {
        checkInitError(taskName, errorTask, retVal < 0);
    }

    void sdlCheckErrorPtr(void * retVal, const char * taskName, TaskOnError errorTask) {
        checkInitError(taskName, errorTask, retVal == nullptr);
    }
}