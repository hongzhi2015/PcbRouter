#include "GridPath.h"

void GridPath::removeRedundantPoints() {
    if (this->mSegments.size() <= 2) {
        return;
    }

    // std::cout << "All pts:" << std::endl;
    // for (auto pt : this->mSegments) {
    //     std::cout << pt << std::endl;
    // }
    // std::cout << "End of All pts:" << std::endl;

    std::vector<std::list<Location>::iterator> pointsToRemove;
    auto pointIte = ++this->mSegments.begin();
    auto prevPointIte = this->mSegments.begin();
    auto nextPointIte = pointIte;
    ++nextPointIte;

    for (; nextPointIte != this->mSegments.end();) {
        // std::cout << "prev: " << *prevPointIte << ", pt: " << *pointIte << ",next: " << *nextPointIte << std::endl;
        // Watch out the special case for Via over here
        if (pointIte->m_x - prevPointIte->m_x == nextPointIte->m_x - pointIte->m_x &&
            pointIte->m_y - prevPointIte->m_y == nextPointIte->m_y - pointIte->m_y) {
            // std::cerr << *pointIte << "<= Remove this point" << std::endl;
            pointsToRemove.push_back(pointIte);
        } else {
            //std::cerr << *pointIte << std::endl;
        }
        ++pointIte;
        ++nextPointIte;
        ++prevPointIte;
    }

    for (auto &removePt : pointsToRemove) {
        this->mSegments.erase(removePt);
    }
}

double GridPath::getRoutedWirelength() {
    double totalEstWL = 0.0;
    Location prevLocation = this->mSegments.front();

    for (auto &location : this->mSegments) {
        if (prevLocation == location) {
            continue;
        }
        // Sanity Check
        if (location.m_z != prevLocation.m_z &&
            location.m_y != prevLocation.m_y &&
            location.m_x != prevLocation.m_x) {
            std::cerr << __FUNCTION__ << "() Invalid path between location: " << location << ", and prevLocation: " << prevLocation << std::endl;
            continue;
        }

        // Print Segment/Track/Wire
        if (location.m_x != prevLocation.m_x || location.m_y != prevLocation.m_y) {
            totalEstWL += GlobalParam::gridFactor * Location::getDistance2D(prevLocation, location);
        }
        prevLocation = location;
    }

    return totalEstWL;
}

int GridPath::getRoutedNumVias() {
    int totalNumVia = 0;
    Location prevLocation = this->mSegments.front();

    for (auto &location : this->mSegments) {
        if (prevLocation == location) {
            continue;
        }
        // Sanity Check
        if (location.m_z != prevLocation.m_z &&
            location.m_y != prevLocation.m_y &&
            location.m_x != prevLocation.m_x) {
            std::cerr << __FUNCTION__ << "() Invalid path between location: " << location << ", and prevLocation: " << prevLocation << std::endl;
            continue;
        }
        // Print Through Hole Via
        if (location.m_z != prevLocation.m_z) {
            ++totalNumVia;
        }
        prevLocation = location;
    }

    return totalNumVia;
}