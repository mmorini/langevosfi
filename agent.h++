#ifndef AGENT_HPP
#define AGENT_HPP

static const char AGENT_HPP_SCCS_ID[] __attribute__((used)) = "@(#)agent.h++: $Id$";

template<class Agentbase>
class Agent: public Agentbase {
public:
  virtual ~Agent(){}
  explicit Agent(const int&  n):Agentbase(n){}
  Agent(const Agentbase& a):Agentbase(a){}
  Agent(const typename Agentbase::Enum& a): Agentbase(a){}
};
#endif
