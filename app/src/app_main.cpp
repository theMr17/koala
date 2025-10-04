#include "app_layer.h"
#include "application.h"

/**
 * @brief Program entry point that starts the application with a GUI-based async HTTP demo layer.
 */
int main() {
    koala::core::ApplicationSpecs app_spec;
    app_spec.width = 1600;
    app_spec.height = 900;
    app_spec.title = "Koala App";
    app_spec.gl_major = 4;
    app_spec.gl_minor = 6;
    app_spec.vsync = true;

    koala::core::Application application(app_spec);
    application.PushLayer(std::make_shared<AppLayer>());
    application.Run();
    return 0;
}
