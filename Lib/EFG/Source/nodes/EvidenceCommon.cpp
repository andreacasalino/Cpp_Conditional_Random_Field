/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "EvidenceCommon.h"
#include <distribution/factor/const/Factor.h>

namespace EFG::nodes {
    void disconnect(Node& node, std::size_t value) {
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            Node* neighbour = it->first;
            distribution::DistributionPtr factor = it->second.factor;

            neighbour->activeConnections.erase(neighbour->activeConnections.find(&node));
            Connection newConnection(factor, std::make_unique<distribution::factor::cnst::Factor>(*factor, Combination({ value }), categoric::Group(node.variable)));
            neighbour->disabledConnections.emplace(&node, std::move(newConnection));

            node.disabledConnections.emplace(neighbour, factor);

        }
        node.activeConnections.clear();
    }

    void reconnnect(Node& node) {
        for (auto it = node.disabledConnections.begin(); it != node.disabledConnections.end(); ++it) {
            Node* neighbour = it->first;
            distribution::DistributionPtr factor = it->second.factor;

            neighbour->disabledConnections.erase(neighbour->disabledConnections.find(&node));
            neighbour->activeConnections.emplace(&node, factor);

            node.activeConnections.emplace(neighbour, factor);
        }
        node.disabledConnections.clear();
    }
}