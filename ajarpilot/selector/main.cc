#include <iostream>

#include <QApplication>
#include <QScreen>
#include <MainWindow.h>

#ifdef QCOM2
#include <qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>
#include <QPlatformSurfaceEvent>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const QSize size = QGuiApplication::primaryScreen()->size();
    MainWindow w;
    w.resize(size.width() / 2.0f, size.height() / 2.0f);
    w.show();

#ifdef QCOM2
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    wl_surface *s = reinterpret_cast<wl_surface*>(auto native_surface = native->nativeResourceForWindow("surface", w.windowHandle()));
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
