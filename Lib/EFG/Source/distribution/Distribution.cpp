/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
#include <distribution/DistributionIterator.h>
#include <distribution/DistributionFinder.h>
#include <categoric/Range.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    void Distribution::checkCombination(const categoric::Combination& comb, const float& value) const {
        if (value < 0.f) {
            throw Error("negative value is invalid");
        }
        if (comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination size");
        }
        std::size_t k = 0;
        const auto* dataPtr = comb.data();
        std::for_each(this->getGroup().getVariables().begin(), this->getGroup().getVariables().end(), [&](const categoric::VariablePtr& v) {
            if (dataPtr[k] >= v->size()) {
                throw Error("combination value exceed variable domain size");
            }
            ++k;
        });
    }

    float Distribution::find(const categoric::Combination& comb) const {
        return this->evaluator->evaluate(this->findRaw(comb));
    }

    float Distribution::findRaw(const categoric::Combination& comb) const {
        auto it = this->values->find(comb);
        if (it == this->values->end()) return 0.f;
        return it->second;
    }

    DistributionIterator Distribution::getIterator() const {
        return DistributionIterator(*this);
    }

    DistributionFinder Distribution::getFinder(const std::set<categoric::VariablePtr>& containingGroup) const {
        return DistributionFinder(*this, containingGroup);
    }

    std::vector<float> Distribution::getProbabilities() const {
        if(this->values->empty()) {
            return std::vector<float>(this->group->size() , 1.f  /static_cast<float>(this->group->size()));
        }
        std::vector<float> probs;
        const std::size_t jointSize = this->group->size();
        probs.reserve(jointSize);
        if (jointSize == this->values->size()) {
            for (auto it = this->values->begin(); it != this->values->end(); ++it) {
                probs.push_back(this->evaluator->evaluate(it->second));
            }
        }
        else {
            categoric::Range jointDomain(this->group->getVariables());
            iterator::forEach(jointDomain, [this, &probs](categoric::Range& jointDomain) {
                auto it = this->values->find(jointDomain.get());
                if (it == this->values->end()) {
                    probs.push_back(0.f);
                }
                else {
                    probs.push_back(this->evaluator->evaluate(it->second));
                }
            });
        }
        // normalize values
        float sum = 0.f;
        std::for_each(probs.begin(), probs.end(), [&sum](const float& v) { sum += v; });
        std::for_each(probs.begin(), probs.end(), [&sum](float& v) { v /= sum; });
        return probs;
    }
}
