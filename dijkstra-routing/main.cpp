#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;

class CDepot {
public:
    CDepot & road(const string & from, const string & to, unsigned weight) {
        if (weight <= 0) throw std::out_of_range("Weight is 0 or less");

        if (from == to) return *this;
        if (m_graph[from].count(to) >= 1 || m_graph[to].count(from) >= 1) {
            if (weight > m_graph[from][to]) {
                m_graph[from][to] = weight;
            }

            if (weight > m_graph[to][from]) {
                m_graph[to][from] = weight;
            }

            return *this;
        }

        m_graph[from][to] = weight;
        m_graph[to][from] = weight;

        return *this;
    }

    CDepot & optimize() {
        return *this;
    }

    unsigned maxWeight(const vector<string> & depots, unsigned reqCities) const {
        std::priority_queue<std::pair<unsigned, std::string>> pq;
        std::unordered_set<std::string> visited;

        for (const auto & depo : depots) {
            if (m_graph.count(depo) == 0)
                throw logic_error("Invalid depot");

            pq.emplace(UINT_MAX, depo);
        }

        unsigned processed = 0;
        unsigned minWeight = UINT_MAX;

        while (!pq.empty()) {
            auto [weight, city] = pq.top();
            pq.pop();

            if (visited.count(city) >= 1) continue;

            visited.insert(city);

            processed++;

            minWeight = std::min(weight, minWeight);

            if (processed >= reqCities) break;

            for (const auto & neighbour : m_graph.at(city)) {
                if (visited.count(neighbour.first) == 0) {
                    pq.emplace(std::min(neighbour.second, weight), neighbour.first);
                }
            }
        }

        if (processed < reqCities)
            throw logic_error("Not enough reachable cities");

        return minWeight;
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned>> m_graph;
    const unsigned UINT_MAX = numeric_limits<uint>().max();
};

int main() {
  CDepot d;
  const unsigned UINT_MAX = numeric_limits<uint>().max();

  d.road("Praha", "Brno", 12)
  .road("Praha", "Brno", 6)
  .road("Praha", "Usti", 5)
  .road("Praha", "Plzen", 7)
  .road("Praha", "Pardubice", 9)
  .road("Jihlava", "Plzen", 10)
  .road("Jihlava", "Ceske Budejovice", 8)
  .road("Jihlava", "Brno", 5)
  .road("Brno", "Zlin", 14)
  .road("Brno", "Ostrava", 4)
    .road("Pardubice", "Ostrava", 6)
  .road("Praha", "Praha", 15)
  .optimize();

  assert(d.maxWeight({"Praha"}, 1) == UINT_MAX);
  assert(d.maxWeight({"Praha"}, 5) == 7);
  assert(d.maxWeight({"Praha"}, 4) == 9);
  assert(d.maxWeight({"Ostrava"}, 2) == 6);
  assert(d.maxWeight({"Brno"}, 9) == 5);
  assert(d.maxWeight({"Plzen", "Jihlava"}, 3) == 8);
  assert(d.maxWeight({"Ostrava", "Zlin"}, 6) == 7);
  assert(d.maxWeight({"Zlin", "Ceske Budejovice"}, 3) == 14);
  assert(d.maxWeight({"Pardubice", "Plzen", "Ostrava"}, 6) == 9);
  assert(d.maxWeight({"Pardubice", "Plzen", "Ostrava"}, 3) == UINT_MAX);
  assert(d.maxWeight({"Usti", "Ostrava", "Jihlava", "Ceske Budejovice"}, 7) == 7);

  // Invalid depot
  try
  {
    d.maxWeight({"Incorrect city"}, 5);
    assert ( "No invalid_argument exception caught!" == nullptr );
  }
  catch ( const logic_error & e ) {}

  // More reqCities than depots
  try
  {
    d.maxWeight({"Praha"}, 10);
    assert ( "No invalid_argument exception caught!" == nullptr );
  }
  catch ( const logic_error & e ) {}

  // Not connected
  d.road("Liberec", "Hradec Kralove", 3)
  .optimize();

  assert(d.maxWeight({"Liberec"}, 1) == UINT_MAX);
  assert(d.maxWeight({"Liberec"}, 2) == 3);

  // Not enough reachable cities
  try
  {
    d.maxWeight({"Liberec"}, 3);
    assert ( "No invalid_argument exception caught!" == nullptr );
  }
  catch ( const logic_error & e ) {}


  std::cout << "All asserts passed" << std::endl;

  return EXIT_SUCCESS;
}

