/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BELIEF_AWARE_H
#define EFG_NODES_BELIEF_AWARE_H

#include <nodes/Node.h>
#include <nodes/bases/Base.h>
#include <memory>

namespace EFG::nodes {
    enum PropagationKind { Sum, MAP };

    struct PropagationResult {
        PropagationKind kindDone;
        //std::size_t iterationsRequired;
        bool wasTerminated;
    };

    class BeliefAware : virtual public Base {
    public:
        void setPropagationMaxIterationsLoopyPropagation(std::size_t iterations) { this->maxIterationsLoopyPropagtion = iterations; };
        inline std::size_t getPropagationMaxIterationsLoopyPropagation() const { return this->maxIterationsLoopyPropagtion; }

        inline const PropagationResult* getLastPropagationResult() const {return this->lastPropagation.get(); };

    protected:
        virtual void propagateBelief(const PropagationKind& kind) = 0;

        std::size_t maxIterationsLoopyPropagtion = 1000;
        std::unique_ptr<PropagationResult> lastPropagation;
    };
}

#endif
