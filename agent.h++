#ifndef AGENT_HPP
#define AGENT_HPP

#include "sccs.h++"

namespace Agent {

static const SCCS::sccs_id AGENT_HPP_SCCS_ID __attribute__((used)) = "@(#)agent.h++: $Id$";

template<class Agentbase>
class Agent: public Agentbase {
public:
  virtual ~Agent() override = default;
  using Agentbase::Agentbase;
  constexpr Agent(const Agentbase& a):Agentbase(a){}
};

}
#endif
