#pragma once

#include <string>

namespace netopt2 {

// A single shipment that must be loaded onto exactly one truck.
class Shipment {
public:
    Shipment(int id, double weight, double volume)
        : id_(id), weight_(weight), volume_(volume) {}

    int id() const { return id_; }
    double weight() const { return weight_; }
    double volume() const { return volume_; }

private:
    int id_;
    double weight_;
    double volume_;
};

} // namespace netopt2
