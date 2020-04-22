#include "MultipinRoute.h"

double MultipinRoute::getRoutedWirelength() const {
    double routedWL = 0.0;
    for (const auto &gp : this->mGridPaths) {
        routedWL += gp.getRoutedWirelength();
    }
    return routedWL;
}

int MultipinRoute::getRoutedNumVias() const {
    int numRoutedVias = 0;
    for (const auto &gp : this->mGridPaths) {
        numRoutedVias += gp.getRoutedNumVias();
    }
    return numRoutedVias;
}

int MultipinRoute::getRoutedNumBends() const {
    int numRoutedBends = 0;
    for (const auto &gp : this->mGridPaths) {
        numRoutedBends += gp.getRoutedNumBends();
    }
    return numRoutedBends;
}

void MultipinRoute::gridPathSegmentsToLocations() {
    for (auto &&gp : this->mGridPaths) {
        gp.transformSegmentsToLocations();
    }
}

void MultipinRoute::gridPathLocationsToSegments() {
    // 1. Copy GridPath's Locations into Segments
    for (auto &&gp : this->mGridPaths) {
        gp.copyLocationsToSegments();
    }

    // 2. Remove Redundant points in paths
    for (auto &&gp : this->mGridPaths) {
        gp.removeRedundantPoints();
    }
}

void MultipinRoute::removeFirstGridPathRedudantLocations() {
    if (this->mGridPaths.size() != 3) {
        if (GlobalParam::gVerboseLevel <= VerboseLevel::WARNING) {
            std::cout << __FUNCTION__ << "(): Supports only 3 gridpaths and removes redundant locations in the first grid path." << std::endl;
        }
        return;
    }

    auto &gpToModify = this->mGridPaths.front();
    auto &gpLocations = gpToModify.setLocations();
    for (unsigned int i = 1; i < this->mGridPaths.size(); ++i) {
        auto &gp = this->mGridPaths.at(i);
        const Location &endOnFirstGp = gp.getLocations().back();

        int disToBack = abs(gpLocations.back().x() - endOnFirstGp.x()) + abs(gpLocations.back().y() - endOnFirstGp.y());
        int disToFront = abs(gpLocations.front().x() - endOnFirstGp.x()) + abs(gpLocations.front().y() - endOnFirstGp.y());

        if (disToBack < disToFront) {
            // std::cout << "endOnFirstGp: " << endOnFirstGp << ", gpLocations.back(): " << gpLocations.back() << std::endl;
            // Remove locations in gpLocations from the back
            while (gpLocations.back() != endOnFirstGp) {
                gpLocations.pop_back();
            }
        } else {
            // std::cout << "endOnFirstGp: " << endOnFirstGp << ", gpLocations.back(): " << gpLocations.front() << std::endl;
            // Remove locations in gpLocations from the front
            while (gpLocations.front() != endOnFirstGp) {
                gpLocations.pop_front();
            }
        }
    }
}

void MultipinRoute::featuresToGridPaths() {
    if (this->features.empty() || this->features.size() == 1) {
        cerr << __FUNCTION__ << "(): No features to translate to segments. Features.size(): " << this->features.size() << std::endl;
        return;
    }

    // std::cout << "Starting of " << __FUNCTION__ << "() ..." << std::endl;

    // Clean up
    this->mGridPaths.clear();

    if (this->features.size() == 2) {
        auto &&path = this->getNewGridPath();
        for (const auto &location : this->features) {
            path.mSegments.push_back(location);
        }
    }

    // Handle this->features.size() > 2
    // New start of a path
    this->mGridPaths.push_back(GridPath{});
    mGridPaths.back().mSegments.push_back(this->features.front());

    // Debuging
    // for (const auto &feature : this->features) {
    //     std::cout << feature << std::endl;
    // }
    // Debugging
    // double estGridWL = 0.0;

    // 1. Separate the features into paths
    for (int i = 1; i < this->features.size(); ++i) {
        const auto &prevLocation = this->features.at(i - 1);
        const auto &location = this->features.at(i);

        // if (abs(location.m_x - prevLocation.m_x) <= 1 &&
        //     abs(location.m_y - prevLocation.m_y) <= 1 &&
        //     abs(location.m_z - prevLocation.m_z) <= 1) {
        //     // Sanity Check
        //     if (location.m_z != prevLocation.m_z &&
        //         location.m_y != prevLocation.m_y &&
        //         location.m_x != prevLocation.m_x) {
        //         std::cerr << __FUNCTION__ << "() Invalid path between location: " << location << ", and prevLocation: " << prevLocation << std::endl;
        //         continue;
        //     }
        if ((abs(location.m_x - prevLocation.m_x) <= 1 && abs(location.m_y - prevLocation.m_y) <= 1 && location.m_z == prevLocation.m_z) ||
            (location.m_x == prevLocation.m_x && location.m_y == prevLocation.m_y && location.m_z != prevLocation.m_z)) {
            // If (is trace or is via)
            mGridPaths.back().mSegments.emplace_back(location);

            // // Debugging
            // if (abs(location.m_x - prevLocation.m_x) == 1 && abs(location.m_y - prevLocation.m_y) == 1) {
            //     estGridWL += GlobalParam::gDiagonalCost;
            // } else if (abs(location.m_x - prevLocation.m_x) == 1 || abs(location.m_y - prevLocation.m_y) == 1) {
            //     estGridWL += 1.0;
            // }
        } else {
            // New start of a path
            this->mGridPaths.push_back(GridPath{});
            mGridPaths.back().mSegments.emplace_back(location);
        }
    }

    // std::cout << __FUNCTION__ << "(): # paths: " << this->mGridPaths.size() << ", estimated Grid WL: " << estGridWL << std::endl;

    // 2. Remove Redundant points in paths
    for (auto &&path : this->mGridPaths) {
        path.removeRedundantPoints();
    }

    // std::cout << "End of " << __FUNCTION__ << "()" << std::endl;
}
