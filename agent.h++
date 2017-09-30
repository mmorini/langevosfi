#ifndef AGENT_HPP
#define AGENT_HPP

namespace Agent {

static const char AGENT_HPP_SCCS_ID[] __attribute__((used)) = "@(#)agent.h++: $Id$";

template<class Agentbase>
class Agent: public Agentbase {
public:
  virtual ~Agent() = default;
  using Agentbase::Agentbase;
  constexpr Agent(const Agentbase& a):Agentbase(a){}
};

}
#endif
