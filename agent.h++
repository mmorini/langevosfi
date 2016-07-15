#ifndef AGENT_HPP
#define AGENT_HPP

static const char AGENT_HPP_SCCS_ID[] __attribute__((used)) = "@(#)agent.h++: $Id$";

class Agent: public Agentbase {
public:
  explicit Agent(const int&  n):Agentbase(n){}
  Agent(const Agentbase& a):Agentbase(a){}
  Agent(const Enum& a): Agentbase(a){}
};
#endif
