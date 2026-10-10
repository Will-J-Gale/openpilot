#include <kj/io.h>
#include <capnp/serialize-packed.h>
#include "ajarpilot/ui/src/model_renderer.h"
#include "ajarpilot/ui/src/ui.h"

ModelRenderer::ModelRenderer()
{
  sub = std::make_unique<emessgee::Subscriber>("model_data");
}

void ModelRenderer::set_video_transform(Eigen::Matrix3f& transform)
{
  video_transform = transform;
}

void ModelRenderer::render(QPainter& painter)
{
  emessgee::ReadResult result = sub->recv("model_data");
  if(result.valid)
  {
    kj::ArrayInputStream input_stream(kj::arrayPtr(result.data, result.size));
    capnp::PackedMessageReader reader(input_stream);
    ModelData::Reader model_data = reader.getRoot<ModelData>();

    render_path(painter, model_data);
    render_lead_car(painter, model_data);
  }
}

void ModelRenderer::render_path(QPainter& painter, ModelData::Reader& model_data)
{
  painter.save();
  float image_width = painter.device()->width();
  float image_height = painter.device()->height();

  QPolygonF polygon;
  polygon << world_to_image(Eigen::Vector3f(PATH_WIDTH, CAMERA_HEIGHT, Z_OFFSCREEN), video_transform);

  for(size_t i = 0; i < model_data.getPath().size(); i++)
  {
    Eigen::Vector3f world_point(
      model_data.getPath()[i].getX() + PATH_WIDTH,
      model_data.getPath()[i].getY() + CAMERA_HEIGHT,
      model_data.getPath()[i].getZ()
    );

    QPointF image_point = world_to_image(world_point, video_transform);

    if(not point_valid(image_point, image_width, image_height))
    {
      continue;
    }

    polygon << image_point;
  }

  for(size_t i = model_data.getPath().size() - 1; i > 0; i--)
  {
    Eigen::Vector3f world_point(
      model_data.getPath()[i].getX() - PATH_WIDTH,
      model_data.getPath()[i].getY() + CAMERA_HEIGHT,
      model_data.getPath()[i].getZ()
    );

    QPointF image_point = world_to_image(world_point, video_transform);

    if(not point_valid(image_point, image_width, image_height))
    {
      continue;
    }

    polygon << image_point;
  }

  polygon << world_to_image(Eigen::Vector3f(-PATH_WIDTH, CAMERA_HEIGHT, Z_OFFSCREEN), video_transform);

  painter.setPen(Qt::NoPen);
  painter.setBrush(PATH_COLOUR);
  painter.drawPolygon(polygon);
  painter.restore();
}

void ModelRenderer::render_lead_car(QPainter& painter, ModelData::Reader& model_data)
{
  painter.save();
  auto lead_car = model_data.getLeadCar();
  painter.setPen(QPen(PATH_COLOUR, 2));
  painter.setBrush(lerp_colour(LEAD_CAR_LOW_CONFIDENCE, LEAD_CAR_HIGH_CONFIDENCE, lead_car[0]));

  if(lead_car[0] >= LEAD_CAR_THRESH)
  {
    float image_width = painter.device()->width();
    float image_height = painter.device()->height();
    Eigen::Vector3f world_point(lead_car[1], lead_car[2] + CAMERA_HEIGHT, lead_car[3]);
    QPointF image_point = world_to_image(world_point, video_transform);

    if(point_valid(image_point, image_width, image_height))
    {
      QPolygonF polygon;
      polygon << image_point;
      polygon << QPointF(image_point.x() - LEAD_CAR_SIZE, image_point.y() + LEAD_CAR_SIZE);
      polygon << QPointF(image_point.x() + LEAD_CAR_SIZE, image_point.y() + LEAD_CAR_SIZE);
      painter.drawPolygon(polygon);
    }
  }

  painter.restore();
}

bool ModelRenderer::point_valid(QPointF p, float image_width, float image_height)
{
  if(
      std::isinf(p.x()) or std::isnan(p.x())
      or std::isinf(p.y()) or std::isnan(p.y())
      or p.x() < 0 or p.x() > image_width
      or p.y() < 0 or p.y() > image_height)
    {
      return false;
    }

    return true;
}