/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <node/belprop/BasicPropagator.h>
#include <cmath>
#include <float.h>
#include "../NeighbourConnection.h"
#include <algorithm>
#include <set>
using namespace std;

namespace EFG::node::bp {

	bool BasicStrategy::operator()(std::list<std::unordered_set<EFG::node::Node*>>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations, thpl::equi::Pool* pl) {

		bool flag = true;
		for (auto it = cluster.begin(); it != cluster.end(); ++it) {
			if (!this->MessagePassing(pl, *it, sum_or_MAP)) {
				if (!this->LoopyPropagation(pl, *it, sum_or_MAP, max_iterations)) flag = false;
			}
		}
		return flag;

	};



	bool BasicStrategy::MessagePassing(thpl::equi::Pool* pool, std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP) {

		list<Node::NeighbourConnection*> open_set;
		auto it = cluster.begin();
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator it_a;
		for (it; it != it_end; ++it) {
			for (it_a = (*it)->GetActiveConnections()->begin(); it_a != (*it)->GetActiveConnections()->end(); ++it_a) open_set.push_back(*it_a);
		}

		bool advance_done;
		list<Node::NeighbourConnection*>::iterator it_o;
		if (pool == nullptr) {
			while (!open_set.empty()) {
				advance_done = false;
				it_o = open_set.begin();
				while (it_o != open_set.end()) {
					if ((*it_o)->isOutgoingRecomputationPossible()) {
						advance_done = true;
						(*it_o)->RecomputeOutgoing(sum_or_MAP);
						it_o = open_set.erase(it_o);
					}
					else ++it_o;
				}
				if (!advance_done) return false;
			}
		}
		else {
			while (!open_set.empty()) {
				advance_done = false;
				it_o = open_set.begin();
				while (it_o != open_set.end()) {
					if ((*it_o)->isOutgoingRecomputationPossible()) {
						advance_done = true;
						Node::NeighbourConnection* temp = *it_o;
						pool->push([temp, &sum_or_MAP]() { temp->RecomputeOutgoing(sum_or_MAP); });
						it_o = open_set.erase(it_o);
					}
					else ++it_o;
				}
				pool->wait();
				if (!advance_done) return false;
			}
		}
		return true;

	}



	bool BasicStrategy::LoopyPropagation(thpl::equi::Pool* pool, std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) {

		//init message not computed
		list<Node::NeighbourConnection*>  mex_to_calibrate;
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator itc, itc_end;
		for (auto it = cluster.begin(); it != it_end; ++it) {
			itc_end = (*it)->GetActiveConnections()->end();
			for (itc = (*it)->GetActiveConnections()->begin(); itc != itc_end; ++itc) {
				//if((*itc)->Get_Linked()->Get_IncomingMessage() == nullptr){
				(*itc)->GetLinked()->SetIncoming2Ones();
				mex_to_calibrate.push_back((*itc)->GetLinked());
				//}
			}
		}

		if (pool == nullptr) {
			float max_variation, temp;
			auto recalibrate_all = [&max_variation, &temp, &sum_or_MAP](auto mex_to_calibrate) {
				std::for_each(mex_to_calibrate.begin(), mex_to_calibrate.end(), [&max_variation, &temp, &sum_or_MAP](Node::NeighbourConnection* c) {
					temp = c->RecomputeOutgoing(sum_or_MAP);
					if (temp > max_variation) max_variation = temp;
				});
			};

			for (unsigned int k = 0; k < max_iterations; ++k) {
				max_variation = 0.f; //it's a positive quantity
				recalibrate_all(mex_to_calibrate);
				if (max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
			}
		}
		else {
			float max_variation;
			list<Node::NeighbourConnection*> mex_remaining, mex_to_recompute;
			set<const pot::IPotential*> mex_locked; //the ones involved in the recomputation of another message
			for (unsigned int k = 0; k < max_iterations; ++k) {
				max_variation = 0.f; //it's a positive quantity
				mex_remaining = mex_to_calibrate;
				//recompute all the messages
				while (!mex_remaining.empty()) {
					mex_locked.clear();
					mex_to_recompute.clear();
					auto rr = mex_remaining.begin();
					while (rr != mex_remaining.end()) {
						// check the message to recompute is not locked
						if (mex_locked.find((*rr)->GetLinked()->GetIncomingMessage()) == mex_locked.end()) {
							mex_to_recompute.push_back(*rr);
							// add the neighbour to the locked message
							auto required = (*rr)->GetNeighbourhood();
							for (auto nn = required->begin(); nn != required->end(); ++nn) {
								if (mex_locked.find((*nn)->GetIncomingMessage()) == mex_locked.end()) mex_locked.emplace((*nn)->GetIncomingMessage());
							}
							rr = mex_remaining.erase(rr);
						}
						else ++rr;
					}
					std::for_each(mex_remaining.begin(), mex_remaining.end(), [&max_variation, &sum_or_MAP, &pool](Node::NeighbourConnection* c) {
						pool->push([&max_variation, &sum_or_MAP, c]() {
							float temp = c->RecomputeOutgoing(sum_or_MAP);
							if (temp > max_variation) max_variation = temp;
						});
					});
					pool->wait();
				}
				if (max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
			}
		}
		return false;

	}

}