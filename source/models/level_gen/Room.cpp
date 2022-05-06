#include "Room.h"

#include "../../generators/Hungarian.h"

#pragma mark Room

namespace level_gen {

Room::Room(default_rooms::RoomConfig config)
    : _type(RoomType::STANDARD),
      _visited(false),
      _fixed(false),
      _scene2_key(""),
      _scene2_source(config.scene2_source),
      _num_players_for_terminal(0),
      _to_delete(false),
      _key(-1) {
  for (cugl::Vec2 &door : config.doors) {
    // If the door is within the bounds of the edges of the room then add it to
    // the rooms doors.
    if ((door.x == 0.0f || door.x == config.size.width - 1) &&
        (door.y > 0.0f && door.y < config.size.height - 1)) {
      _doors.push_back(door);
    }

    if ((door.y == 0.0f || door.y == config.size.height - 1) &&
        (door.x > 0.0f && door.x < config.size.width - 1)) {
      _doors.push_back(door);
    }
  }

  initScene2(config.size);
}

void Room::move(cugl::Vec2 dist) {
  if (!_fixed) {
    dist.set(roundf(dist.x), roundf(dist.y));
    _node->setPosition(_node->getPosition() + dist);
  }
}

void Room::moveTo(cugl::Vec2 pos) {
  if (!_fixed) _node->setPosition(pos);
}

void Room::addEdge(const std::shared_ptr<Edge> &edge) {
  auto it = std::upper_bound(
      _edges.begin(), _edges.end(), edge,
      [this](const std::shared_ptr<Edge> &l, const std::shared_ptr<Edge> &r) {
        return edgeComparator(l, r);
      });
  _edges.insert(it, edge);
}

bool Room::edgeComparator(const std::shared_ptr<Edge> &l,
                          const std::shared_ptr<Edge> &r) const {
  int l_side_idx = l->getSideIndex(getRect());
  int r_side_idx = r->getSideIndex(getRect());

  if (l_side_idx != r_side_idx) return l_side_idx < r_side_idx;

  cugl::Vec2 l_disp = (l->_path[0] == getMid()) ? l->_path[1] - l->_path[0]
                                                : l->_path[0] - l->_path[1];
  cugl::Vec2 r_disp = (r->_path[0] == getMid()) ? r->_path[1] - r->_path[0]
                                                : r->_path[0] - r->_path[1];

  float l_angle = l_disp.getAngle();
  float r_angle = r_disp.getAngle();

  if (l_side_idx == 0) return l_angle < r_angle;

  l_angle += (l_angle < 0) ? 2.0f * M_PI : 0.0f;
  r_angle += (r_angle < 0) ? 2.0f * M_PI : 0.0f;

  return l_angle < r_angle;
}

bool Room::hasNeighbor(const std::shared_ptr<Room> &room) const {
  return findEdge(room) != nullptr;
}

std::shared_ptr<Edge> Room::findEdge(const std::shared_ptr<Room> &room) const {
  auto result = std::find_if(
      _edges.begin(), _edges.end(), [room](const std::shared_ptr<Edge> &edge) {
        return edge->_source == room || edge->_neighbor == room;
      });

  return (result != _edges.end()) ? *result : nullptr;
}

cugl::Vec2 Room::getDoorForEdge(const std::shared_ptr<Edge> &edge) {
  if (std::find(_edges.begin(), _edges.end(), edge) == _edges.end())
    return cugl::Vec2::ONE * -1.0f;
  if (_edge_to_door.size() == 0) initializeEdgeToDoorPairing();
  return _edge_to_door[edge];
}

void Room::initializeEdgeToDoorPairing() {
  std::vector<std::vector<double>> weight_matrix;
  std::vector<std::shared_ptr<Edge>> active_edges;
  for (std::shared_ptr<Edge> &edge : _edges) {
    if (edge->_active) {
      active_edges.push_back(edge);
      std::vector<double> weights;
      for (cugl::Vec2 &door : _doors) {
        weights.push_back(angleBetweenEdgeAndDoor(edge, door));
      }
      weight_matrix.push_back(weights);
    }
  }

  HungarianAlgorithm HungAlgo;
  std::vector<int> assignment;
  HungAlgo.Solve(weight_matrix, assignment);

  for (int i = 0; i < weight_matrix.size(); i++)
    _edge_to_door[active_edges[i]] = _doors[assignment[i]];
}

float Room::angleBetweenEdgeAndDoor(const std::shared_ptr<Edge> &edge,
                                    cugl::Vec2 door) {
  cugl::Vec2 approx_door = door;
  if (approx_door.x == 0 || approx_door.x == _size.width - 1)
    approx_door.y = (int)(_size.height / 2);
  else if (approx_door.y == 0 || approx_door.y == _size.height - 1)
    approx_door.x = (int)(_size.width / 2);

  cugl::Rect rect = getRect();
  cugl::Vec2 mid = getMid();
  cugl::Vec2 intersect = edge->getIntersectWithRectSide(rect);
  float edge_angle = (intersect - mid).getAngle();
  float door_angle = (door + rect.origin - mid).getAngle();
  float approx_door_angle = (approx_door + rect.origin - mid).getAngle();
  float diff = fabs(door_angle - edge_angle);
  float approx_diff = fabs(approx_door_angle - edge_angle);
  if (diff > M_PI) diff = 2 * M_PI - diff;
  if (approx_diff > M_PI) approx_diff = 2 * M_PI - approx_diff;
  return std::min(diff, approx_diff);
}

void Room::initScene2(cugl::Size size) {
  _node = cugl::scene2::PolygonNode::alloc();
  _node->setContentSize(size);
  _size = size;

  for (cugl::Vec2 door : _doors) {
    cugl::Vec2 pos(floorf(door.x), floorf(door.y));
    cugl::PolyFactory poly_factory;
    cugl::Poly2 poly_rect = poly_factory.makeRoundedRect(
        cugl::Vec2::ZERO, cugl::Vec2::ONE * 0.9f, 0.1f);
    auto tile = cugl::scene2::PolygonNode::allocWithPoly(poly_rect);
    tile->setColor(cugl::Color4::ORANGE);
    pos += cugl::Vec2(0.05f, 0.05f);
    tile->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    tile->setPosition(pos);
    _node->addChild(tile);
  }

  _node->doLayout();
}

#pragma mark Edge

Edge::Edge(const std::shared_ptr<Room> &s, const std::shared_ptr<Room> &n)
    : _weight(0), _calculated(false) {
  _source = s;
  _neighbor = n;

  _path.push_back(_source->getMid());
  _path.push_back(_neighbor->getMid());

  _weight = (_path[1] - _path[0]).length();

  _node = cugl::scene2::PathNode::allocWithVertices(_path, 0.4f);
  _node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
  _node->setPosition(_node->getBoundingRect().origin);
  _node->setColor(cugl::Color4(0, 0, 0, 50));
}

bool Edge::doesIntersect(cugl::Vec2 origin, float r) {
  cugl::Vec2 d = _neighbor->getMid() - _source->getMid();
  cugl::Vec2 f = _source->getMid() - origin;

  float a = d.dot(d);
  float b = 2 * f.dot(d);
  float c = f.dot(f) - r * r;

  float discriminant2 = b * b - 4 * a * c;
  if (discriminant2 < 0) return false;

  float discriminant = sqrt(discriminant2);

  float t1 = (-1 * b - discriminant) / (2 * a);
  float t2 = (-1 * b + discriminant) / (2 * a);

  return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
}

int Edge::getSideIndex(const cugl::Rect &rect) const {
  // Vertices in clockwise order with right most face first.
  std::vector<cugl::Vec2> vertices{cugl::Vec2(rect.getMaxX(), rect.getMinY()),
                                   cugl::Vec2(rect.getMaxX(), rect.getMaxY()),
                                   cugl::Vec2(rect.getMinX(), rect.getMaxY()),
                                   cugl::Vec2(rect.getMinX(), rect.getMinY()),
                                   cugl::Vec2(rect.getMaxX(), rect.getMinY())};

  for (int i = 0; i < vertices.size() - 1; i++) {
    float S, T;
    bool intersects = cugl::Vec2::doesLineIntersect(
        _path[0], _path[1], vertices[i], vertices[i + 1], &S, &T);

    // Make sure that the we're checking the segments and not the lines made
    // by the points. S & T must be in range [0,1].
    if (intersects && (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f))
      return i;
  }
  return -1;
}

cugl::Vec2 Edge::getIntersectWithRectSide(const cugl::Rect &rect) const {
  // Vertices in clockwise order with right most face first.
  std::vector<cugl::Vec2> vertices{cugl::Vec2(rect.getMaxX(), rect.getMinY()),
                                   cugl::Vec2(rect.getMaxX(), rect.getMaxY()),
                                   cugl::Vec2(rect.getMinX(), rect.getMaxY()),
                                   cugl::Vec2(rect.getMinX(), rect.getMinY()),
                                   cugl::Vec2(rect.getMaxX(), rect.getMinY())};

  for (int i = 0; i < vertices.size() - 1; i++) {
    float S, T;
    bool intersects = cugl::Vec2::doesLineIntersect(
        _path[0], _path[1], vertices[i], vertices[i + 1], &S, &T);

    // Make sure that the we're checking the segments and not the lines made
    // by the points. S & T must be in range [0,1].
    if (intersects && (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f))
      return _path[0] + (_path[1] - _path[0]) * S;
  }
  return cugl::Vec2::ZERO;
}

}  // namespace level_gen
