#pragma once
#include <iostream>

#define MAX_STATES  100

using namespace std;

class Condition
{
public:
   virtual bool check(HybridAutomata* HA, int *value) = 0;
};

class HybridAutomata
{
public:
   unsigned int curState;
   void setState(unsigned int id, void (*ah)()); // user sets states
   void setCondition(Condition *cDo, unsigned int preState, unsigned int postState); //user sets conditions
   void setCondition(unsigned int preState, unsigned int postState);
   void operate();// check conditions and if condition has satisfied move to another state
   void operate(unsigned int post);
   HybridAutomata(unsigned int init, unsigned int exit);
   HybridAutomata();
   ~HybridAutomata();
private:
   
   class State
   {
   public:
      unsigned int stateId;
      void (*aDo)();
      State(unsigned int id, void (*ah)())
      {
         stateId = id;
         aDo = ah;
      }
   };
   unsigned int initState, exitState;
   unsigned int nStates;
   bool stateMachine[MAX_STATES][MAX_STATES];
   State *states[MAX_STATES];
   Condition *conditions[MAX_STATES][MAX_STATES];
   void initStateMachineArr();
   void initConditionsArr();
   void initStateArr();
   bool checkStateMachine(unsigned int pre, unsigned int post);
   int checkConditions();
   int checkConditions(unsigned int post);
};
