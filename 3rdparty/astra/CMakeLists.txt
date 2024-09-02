project(astra)
set(SRC
        ./astra/hal/hal_dreamCore/components/hal_buzzer.cpp
        ./astra/hal/hal_dreamCore/components/oled/hal_oled.cpp
        ./astra/hal/hal_dreamCore/components/hal_config.cpp
        ./astra/hal/hal_dreamCore/components/hal_linux.cpp
        ./astra/hal/hal.cpp
        ./astra/ui/item/plugin/plugin.cpp
        ./astra/ui/item/camera/camera.cpp
        ./astra/ui/item/menu/menu.cpp
        ./astra/ui/item/selector/selector.cpp
        ./astra/ui/item/widget/widget.cpp
        ./astra/ui/launcher.cpp
        ./astra/app/astra_app.cpp
)
include_directories(include)

add_library(${PROJECT_NAME} STATIC ${SRC})