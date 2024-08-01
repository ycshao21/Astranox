#pragma once

#include "Application.hpp"

/*
 * This header file defines the entry point of the application.
 */


#ifdef AW_PLATFORM_WINDOWS

    extern Astranox::Application* Astranox::createApplication();

    int main(int argc, char** argv)
    {
        Astranox::Logging::init();

        Astranox::Application* app = Astranox::createApplication();
        app->run();
        delete app;

        Astranox::Logging::destroy();
    }

#endif

