#include <iostream>

#include <QApplication>
#include <QScreen>
#include <MainWindow.h>

#ifdef QCOM2
#include <qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>
#include <QPlatformSurfaceEvent>
#endif

#include "system/hardware/hw.h"

const QSize DEVICE_SCREEN_SIZE = {2160, 1080};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const QSize size = QGuiApplication::primaryScreen()->size();
    MainWindow w;

    if (Hardware::PC()) {
        w.setMinimumSize(QSize(640, 480)); // allow resize smaller than fullscreen
        w.setMaximumSize(DEVICE_SCREEN_SIZE);
        w.resize(size);
    }
    else 
        w.setFixedSize(DEVICE_SCREEN_SIZE);

    w.show();

#ifdef QCOM2
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    wl_surface *s = reinterpret_cast<wl_surface*>(native->nativeResourceForWindow("surface", w.windowHandle()));
    wl_surface_set_buffer_transform(s, WL_OUTPUT_TRANSFORM_270);
    wl_surface_commit(s);
    w.showFullScreen();

    // ensure we have a valid eglDisplay, otherwise the ui will silently fail
    void *egl = native->nativeResourceForWindow("egldisplay", w.windowHandle());
    std::cout << "Created egl" << std::endl;
    assert(egl != nullptr);
#endif

    return a.exec();
}
