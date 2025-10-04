#include "application.h"
#include "app_layer.h"

/**
 * @brief Program entry point that starts the application with a GUI-based async HTTP demo layer.
 */
int main() {
    koala::core::Application application;
    application.PushLayer(std::make_shared<AppLayer>());
    application.Run();
    return 0;
}
