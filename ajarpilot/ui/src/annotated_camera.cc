#include <iostream>
#include "ajarpilot/ui/src/annotated_camera.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

#include "common/swaglog.h"
#include <kj/io.h>
#include "selfdrive/ui/qt/util.h"
#include "ajarpilot/ui/src/ui.h"

// Window that shows camera view and variety of info drawn on top
AnnotatedCameraWidget2::AnnotatedCameraWidget2(VisionStreamType type, QWidget *parent)
    : fps_filter(UI_FREQ, 3, 1. / UI_FREQ), CameraWidget("camerad", type, parent) {
  pm = std::make_unique<PubMaster>(std::vector<const char*>{"uiDebug"});

  main_layout = new QVBoxLayout(this);
  main_layout->setMargin(UI_BORDER_SIZE);
  main_layout->setSpacing(0);
}

void AnnotatedCameraWidget2::initializeGL() {
  CameraWidget::initializeGL();
  qInfo() << "OpenGL version:" << QString((const char*)glGetString(GL_VERSION));
  qInfo() << "OpenGL vendor:" << QString((const char*)glGetString(GL_VENDOR));
  qInfo() << "OpenGL renderer:" << QString((const char*)glGetString(GL_RENDERER));
  qInfo() << "OpenGL language version:" << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

  prev_draw_t = millis_since_boot();
  setBackgroundColor(BG_COLOUR);
}

mat4 AnnotatedCameraWidget2::calcFrameMatrix() {
  return CameraWidget::calcFrameMatrix();
}

void AnnotatedCameraWidget2::paintGL() {
  {
    std::lock_guard lk(frame_lock);

    if (frames.empty()) {
      if (skip_frame_count > 0) {
        skip_frame_count--;
        qDebug() << "skipping frame, not ready";
        return;
      }
    } else {
      // skip drawing up to this many frames if we're
      // missing camera frames. this smooths out the
      // transitions from the narrow and wide cameras
      skip_frame_count = 5;
    }

    CameraWidget::setStreamType(VISION_STREAM_ROAD);
    CameraWidget::setFrameId(0);
    CameraWidget::paintGL();
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  float widget_aspect_ratio = (float)width() / height();
  float frame_aspect_ratio =  IMAGE_WIDTH / IMAGE_HEIGHT;

  // ZX is scale of screen width to get to the aspect of the video, if you shrink the video by this much it looks normal
  float zx = std::min(frame_aspect_ratio / widget_aspect_ratio, 1.0f);
  float zy = std::min(widget_aspect_ratio / frame_aspect_ratio, 1.0f);

  // Could probably be made into a matrix
  float shown_width = width() * zx;
  float shown_height = height() * zy;
  float x_scale = shown_width / IMAGE_WIDTH;
  float y_scale = shown_height / IMAGE_HEIGHT;
  float x_offset = (width() / 2.0) - (shown_width / 2.0f);

  Eigen::Matrix3f video_transform =(Eigen::Matrix3f() <<
    x_scale, 0.0f, x_offset,
    0.0f, y_scale, 0.0f,
    0.0f, 0.0f, 1.0f).finished();

  model_renderer.set_video_transform(video_transform);
  model_renderer.render(painter);

  double cur_draw_t = millis_since_boot();
  double dt = cur_draw_t - prev_draw_t;
  double fps = fps_filter.update(1. / dt * 1000);
  if (fps < 15) {
    LOGW("slow frame rate: %.2f fps", fps);
  }
  prev_draw_t = cur_draw_t;

}

void AnnotatedCameraWidget2::showEvent(QShowEvent *event) {
  CameraWidget::showEvent(event);

  // ui_update_params(uiState());
  prev_draw_t = millis_since_boot();
}
