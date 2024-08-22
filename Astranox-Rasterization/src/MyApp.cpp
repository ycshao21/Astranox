#include <Astranox.hpp>
//
//#include "TempLayer.hpp"
//#include "Sandbox.hpp"
#include "Sandbox2D.hpp"

class MyApp : public Astranox::Application
{
public:
    MyApp(const Astranox::ApplicationSpecification& spec)
        : Astranox::Application(spec)
    {
        //pushLayer(new TempLayer());
        //pushLayer(new SandboxLayer());
        pushLayer(new Sandbox2DLayer());
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