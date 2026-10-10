#pragma once

#include <QPainter>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include "emessgee/emessgee.h"
#include "ajarpilot/messaging/log.capnp.h"

class ModelRenderer
{
public:
  ModelRenderer();
  void render(QPainter& painter);
  void set_video_transform(Eigen::Matrix3f& transform);

private:
  void render_path(QPainter& painter, ModelData::Reader& model_data);
  void render_lead_car(QPainter& painter, ModelData::Reader& model_data);
  bool point_valid(QPointF p, float image_width, float image_height);

private:
  std::unique_ptr<emessgee::Subscriber> sub;
  Eigen::Matrix3f video_transform;

};