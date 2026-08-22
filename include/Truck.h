#pragma once

#include <string>

namespace netopt2 {

// A truck TYPE available to the solver. The solver may instantiate as many
// copies ("truck instances") of a given type as needed; `costPerTrip` is
// charged once per instance actually used.
class Truck {
public:
    Truck(int typeId, std::string name, double weightCapacity,
          double volumeCapacity, double costPerTrip)
        : typeId_(typeId),
          name_(std::move(name)),
          weightCapacity_(weightCapacity),
          volumeCapacity_(volumeCapacity),
          costPerTrip_(costPerTrip) {}

    int typeId() const { return typeId_; }
    const std::string& name() const { return name_; }
    double weightCapacity() const { return weightCapacity_; }
    double volumeCapacity() const { return volumeCapacity_; }
    double costPerTrip() const { return costPerTrip_; }

private:
    int typeId_;
    std::string name_;
    double weightCapacity_;
    double volumeCapacity_;
    double costPerTrip_;
};

} // namespace netopt2
