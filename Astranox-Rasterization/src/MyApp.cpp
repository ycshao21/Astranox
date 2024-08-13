#include <Astranox.hpp>

#include "TempLayer.hpp"
#include "Sandbox.hpp"

class MyApp : public Astranox::Application
{
public:
    MyApp(const Astranox::ApplicationSpecification& spec)
        : Astranox::Application(spec)
    {
        //pushLayer(new TempLayer());
        pushLayer(new SandboxLayer());
    }

    virtual ~MyApp() = default;
};

Astranox::Application* Astranox::createApplication()
{
    Astranox::ApplicationSpecification appSpec;
    appSpec.name = "Astranox | Real-time Rasterization";
    appSpec.windowWidth = 1440;
    appSpec.windowHeight = 900;
    appSpec.vsync = true;

    return new MyApp(appSpec);
}