#pragma once
#include <QPointF>

const Eigen::Matrix3f CAM_INTRINSIC_MATRIX = (Eigen::Matrix3f() <<
  2648.0, 0.0, 1928.0 / 2,
  0.0, 2648.0, 1208.0 / 2,
  0.0, 0.0, 1.0).finished();

const QColor BG_COLOUR(0, 0, 0, 0);
const QColor PATH_COLOUR(255, 0, 255, 64);
constexpr float PATH_WIDTH = 1.0f;

const QColor LEAD_CAR_COLOUR(0, 255, 0, 255);
constexpr float LEAD_CAR_THRESH = 0.5f;
constexpr float LEAD_CAR_SIZE = 20.0f;
const QColor LEAD_CAR_LOW_CONFIDENCE(255, 0, 0, 255);
const QColor LEAD_CAR_HIGH_CONFIDENCE(0, 255, 0, 255);

constexpr float CAMERA_HEIGHT = 1.0f;
constexpr float Z_OFFSCREEN = 2.1;
constexpr float IMAGE_WIDTH = 1928;
constexpr float IMAGE_HEIGHT = 1208;

inline QPointF world_to_image(const Eigen::Vector3f& p, Eigen::Matrix3f& video_transform)
{
  auto pt = video_transform * (CAM_INTRINSIC_MATRIX * p); // (world -> homog) -> aspect shift for screen size
 return QPointF(pt.x() / pt.z(), pt.y() / pt.z());
}

inline QColor lerp_colour(const QColor &a, const QColor &b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    return QColor::fromRgbF(
        a.redF()   + (b.redF()   - a.redF())   * t,
        a.greenF() + (b.greenF() - a.greenF()) * t,
        a.blueF()  + (b.blueF()  - a.blueF())  * t,
        a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}