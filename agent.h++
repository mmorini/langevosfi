#ifndef AGENT_HPP
#define AGENT_HPP
class Agent: public Agentbase {
public:
  explicit Agent(const int&  n):Agentbase(n){}
  Agent(const Agentbase& a):Agentbase(a){}
  Agent(const Enum& a): Agentbase(a){}
};
#endif
