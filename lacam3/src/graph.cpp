#include "../include/graph.hpp"

Vertex::Vertex(int _id, int _index, int _x, int _y)
    : id(_id), index(_index), x(_x), y(_y), neighbor()
{
}

Graph::Graph() : V(Vertices()), width(0), height(0), grid_type(GridType::SQUARE), hex_config(HexConfig::ODD_ROW) {}

Graph::~Graph()
{
  for (auto &v : V)
    if (v != nullptr) delete v;
  V.clear();
}

// to load graph
static const std::regex r_height = std::regex(R"(height\s(\d+))");
static const std::regex r_width = std::regex(R"(width\s(\d+))");
static const std::regex r_map = std::regex(R"(map)");

Graph::Graph(const std::string &filename, GridType gt, HexConfig hc) 
    : V(Vertices()), width(0), height(0), grid_type(gt), hex_config(hc)
{
  std::ifstream file(filename);
  if (!file) {
    std::cout << "file " << filename << " is not found." << std::endl;
    return;
  }
  std::string line;
  std::smatch results;

  // read fundamental graph parameters
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();

    if (std::regex_match(line, results, r_height)) {
      height = std::stoi(results[1].str());
    }
    if (std::regex_match(line, results, r_width)) {
      width = std::stoi(results[1].str());
    }
    if (std::regex_match(line, results, r_map)) break;
  }

  U = Vertices(width * height, nullptr);

  // create vertices
  int y = 0;
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();
    for (int x = 0; x < width; ++x) {
      char s = line[x];
      if (s == 'T' or s == '@') continue;  // object
      auto index = width * y + x;
      auto v = new Vertex(V.size(), index, x, y);
      V.push_back(v);
      U[index] = v;
    }
    ++y;
  }
  file.close();

  // create edges
  if (grid_type == GridType::SQUARE) {
    // Original square grid implementation
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        // left
        if (x > 0) {
          auto u = U[width * y + (x - 1)];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // right
        if (x < width - 1) {
          auto u = U[width * y + (x + 1)];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // up
        if (y < height - 1) {
          auto u = U[width * (y + 1) + x];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // down
        if (y > 0) {
          auto u = U[width * (y - 1) + x];
          if (u != nullptr) v->neighbor.push_back(u);
        }
      }
    }
  } else if (grid_type == GridType::HEX && hex_config == HexConfig::ODD_ROW) {
    // Hexagonal grid implementation for odd-row (odd-r) configuration
    // Based on: https://www.redblobgames.com/grids/hexagons/#neighbors-offset
    // Direction vectors:
    // even rows: [[+1, 0], [0, -1], [-1, -1], [-1, 0], [-1, +1], [0, +1]]
    // odd rows:  [[+1, 0], [+1, -1], [0, -1], [-1, 0], [0, +1], [+1, +1]]
    
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        
        if (y % 2 == 0) {  // Even row
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [-1, -1] - Northwest
          if (x > 0 && y > 0) {
            auto n = U[width * (y - 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, +1] - Southwest
          if (x > 0 && y < height - 1) {
            auto n = U[width * (y + 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        } else {  // Odd row
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, -1] - Northeast
          if (x < width - 1 && y > 0) {
            auto n = U[width * (y - 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [+1, +1] - Southeast
          if (x < width - 1 && y < height - 1) {
            auto n = U[width * (y + 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        }
      }
    }
  } else if (grid_type == GridType::HEX && hex_config == HexConfig::ODD_COLUMN) {
    // Hexagonal grid implementation for odd-column (odd-q) configuration
    // Based on: https://www.redblobgames.com/grids/hexagons/#neighbors-offset
    // Direction vectors:
    // even columns: [[+1, 0], [+1, -1], [0, -1], [-1, -1], [-1, 0], [0, +1]]
    // odd columns:  [[+1, +1], [+1, 0], [0, -1], [-1, 0], [-1, +1], [0, +1]]
    
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        
        if (x % 2 == 0) {  // Even column
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, -1] - Northeast
          if (x < width - 1 && y > 0) {
            auto n = U[width * (y - 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, -1] - Northwest
          if (x > 0 && y > 0) {
            auto n = U[width * (y - 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        } else {  // Odd column
          // Direction 0: [+1, +1] - Southeast
          if (x < width - 1 && y < height - 1) {
            auto n = U[width * (y + 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, +1] - Southwest
          if (x > 0 && y < height - 1) {
            auto n = U[width * (y + 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        }
      }
    }
  } else if (grid_type == GridType::HEX && hex_config == HexConfig::EVEN_ROW) {
    // Hexagonal grid implementation for even-row (even-r) configuration
    // Based on: https://www.redblobgames.com/grids/hexagons/#neighbors-offset
    // Direction vectors:
    // even rows: [[+1, 0], [+1, -1], [0, -1], [-1, 0], [0, +1], [+1, +1]]
    // odd rows:  [[+1, 0], [0, -1], [-1, -1], [-1, 0], [-1, +1], [0, +1]]
    
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        
        if (y % 2 == 0) {  // Even row
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, -1] - Northeast
          if (x < width - 1 && y > 0) {
            auto n = U[width * (y - 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [+1, +1] - Southeast
          if (x < width - 1 && y < height - 1) {
            auto n = U[width * (y + 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        } else {  // Odd row
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [-1, -1] - Northwest
          if (x > 0 && y > 0) {
            auto n = U[width * (y - 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, +1] - Southwest
          if (x > 0 && y < height - 1) {
            auto n = U[width * (y + 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        }
      }
    }
  } else if (grid_type == GridType::HEX && hex_config == HexConfig::EVEN_COLUMN) {
    // Hexagonal grid implementation for even-column (even-q) configuration
    // Based on: https://www.redblobgames.com/grids/hexagons/#neighbors-offset
    // Direction vectors:
    // even cols: [[+1, +1], [+1, 0], [0, -1], [-1, 0], [-1, +1], [0, +1]]
    // odd cols:  [[+1, 0], [+1, -1], [0, -1], [-1, -1], [-1, 0], [0, +1]]
    
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        
        if (x % 2 == 0) {  // Even column
          // Direction 0: [+1, +1] - Southeast
          if (x < width - 1 && y < height - 1) {
            auto n = U[width * (y + 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, +1] - Southwest
          if (x > 0 && y < height - 1) {
            auto n = U[width * (y + 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        } else {  // Odd column
          // Direction 0: [+1, 0] - East
          if (x < width - 1) {
            auto n = U[width * y + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 1: [+1, -1] - Northeast
          if (x < width - 1 && y > 0) {
            auto n = U[width * (y - 1) + (x + 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 2: [0, -1] - North
          if (y > 0) {
            auto n = U[width * (y - 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 3: [-1, -1] - Northwest
          if (x > 0 && y > 0) {
            auto n = U[width * (y - 1) + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 4: [-1, 0] - West
          if (x > 0) {
            auto n = U[width * y + (x - 1)];
            if (n != nullptr) v->neighbor.push_back(n);
          }
          
          // Direction 5: [0, +1] - South
          if (y < height - 1) {
            auto n = U[width * (y + 1) + x];
            if (n != nullptr) v->neighbor.push_back(n);
          }
        }
      }
    }
  }
}

int Graph::size() const { return V.size(); }

bool is_same_config(const Config &C1, const Config &C2)
{
  const auto N = C1.size();
  for (size_t i = 0; i < N; ++i) {
    if (C1[i]->id != C2[i]->id) return false;
  }
  return true;
}

uint ConfigHasher::operator()(const Config &C) const
{
  uint hash = C.size();
  for (auto &v : C) {
    hash ^= v->id + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}

std::ostream &operator<<(std::ostream &os, const Vertex *v)
{
  os << v->index;
  return os;
}

std::ostream &operator<<(std::ostream &os, const Config &Q)
{
  for (auto v : Q) os << v << ",";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Paths &paths)
{
  for (auto i = 0; i < paths.size(); ++i) {
    os << i << ":";
    for (auto &v : paths[i]) {
      os << std::setw(4) << v << "->";
    }
    std::cout << std::endl;
  }
  return os;
}
