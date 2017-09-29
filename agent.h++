#ifndef AGENT_HPP
#define AGENT_HPP

static const char AGENT_HPP_SCCS_ID[] __attribute__((used)) = "@(#)agent.h++: $Id$";

namespace Agent {

template<class Agentbase>
class Agent: public Agentbase {
public:
  virtual ~Agent(){}
  explicit constexpr Agent(const int&  n):Agentbase(n){}
  constexpr Agent(const Agentbase& a):Agentbase(a){}
  constexpr Agent(const typename Agentbase::Enum& a): Agentbase(a){}
};

}
#endif
