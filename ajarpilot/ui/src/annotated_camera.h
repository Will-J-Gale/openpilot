#pragma once

#include <QVBoxLayout>
#include <memory>
#include "selfdrive/ui/qt/widgets/cameraview.h"
#include "emessgee/emessgee.h"


class AnnotatedCameraWidget2 : public CameraWidget {
  Q_OBJECT

public:
  explicit AnnotatedCameraWidget2(VisionStreamType type, QWidget* parent = 0);
  // void updateState(const UIState &s);

private:
  QVBoxLayout *main_layout;
  // ModelRenderer model;
  std::unique_ptr<PubMaster> pm;

  int skip_frame_count = 1;
  bool wide_cam_requested = false;

protected:
  void paintGL() override;
  void initializeGL() override;
  void showEvent(QShowEvent *event) override;
  mat4 calcFrameMatrix() override;

  std::unique_ptr<emessgee::Subscriber> sub;

  double prev_draw_t = 1;
  FirstOrderFilter fps_filter;
};
