#include "HybridAutomata.h"

HybridAutomata::HybridAutomata(unsigned int init, unsigned int exit)
{
   if (exit >= MAX_STATES)
   {
      cout << "State is bigger than MAX_STATE" << endl;
      return;
   }
   initState = init;
   curState = initState;
   exitState = exit;
   
   initStateMachineArr();
   initStateArr();
   initConditionsArr();

   nStates = 1;
}
HybridAutomata::HybridAutomata()
{
   initState = 0;
   curState = initState;
   exitState = MAX_STATES;

   initStateMachineArr();
   initStateArr();
   initConditionsArr();

   nStates = 1;
}
void HybridAutomata::initStateArr()
{
   for (int i = 0; i < MAX_STATES; i++)
   {
      states[i] = NULL;
   }
}
void HybridAutomata::initStateMachineArr()
{
   for (int i = 0; i< MAX_STATES; i++)
   {
      for (int j = 0; j<MAX_STATES; j++)
      {
         stateMachine[i][j] = 0;
      }
   }
}
void HybridAutomata::initConditionsArr()
{
   for (int i = 0; i< MAX_STATES; i++)
   {
      for (int j = 0; j<MAX_STATES; j++)
      {
         conditions[i][j] = NULL;
      }
   }
}

void HybridAutomata::setState(unsigned int id, void (*ah)())
{
   if (id > exitState) 
   {
      cout << "State id is bigger than exit State" << endl;
   }

   states[id] = new State(id, ah);
   nStates++;
}

void HybridAutomata::setCondition(Condition *cDo, unsigned int preState, unsigned int postState)
{
   if (preState >= exitState || postState >= exitState || preState<initState || postState <initState)
   {
      cout<<"StateId error"<<endl;
      return;
   }

   bool isEmpty;
   isEmpty = checkStateMachine(preState, postState);
   if (isEmpty == true)
   {
      stateMachine[preState][postState] = 1;
      conditions[preState][postState] = cDo;
   }
   else cout<<"this condition has already declared"<<endl;
}
void HybridAutomata::setCondition(unsigned int preState, unsigned int postState)
{
   if (preState >= exitState || postState >= exitState || preState<initState || postState <initState)
   {
      cout<<"StateId error"<<endl;
      return;
   }

   bool isEmpty;
   isEmpty = checkStateMachine(preState, postState);
   if (isEmpty == true)
   {
      stateMachine[preState][postState] = 1;
   }
   else cout<<"this condition has already declared"<<endl;
}

void HybridAutomata::operate()
{
   curState = checkConditions();
   cout<<"Condition satisfied!"<<endl;
   states[curState]->aDo();
}
void HybridAutomata::operate(unsigned int post)
{
    bool isAvail;
    isAvail = checkConditions(post);
    if(!isAvail == true)
    {
        cout<<"Condition satisfied!"<<endl;
        curState=post;
        states[curState]->aDo();
    }
}
bool HybridAutomata::checkStateMachine(unsigned int pre, unsigned int post)
{
   if (stateMachine[pre][post] == 0) return true;
   else return false;
}
int HybridAutomata::checkConditions(unsigned int post)
{
    bool isEmpty = true;
    isEmpty = checkStateMachine(curState, post);
    return isEmpty;
}

int HybridAutomata::checkConditions()
{
   bool isEmpty;
   bool isAvail = false;
   int idx = 0;
   int temp = 0;
   Condition *cand;
   int candidateArr[MAX_STATES];
   for (int i = initState; i<exitState; i++)
   {
      isEmpty = checkStateMachine(curState, i);
      if (isEmpty == false)
      {
         candidateArr[idx] = i;
         idx++;
      }
   }
   while (true)
   {
      temp = temp % (idx + 1);

      cand = conditions[curState][candidateArr[temp]];
      isAvail = cand->check(this);
      if (isAvail == true) return candidateArr[temp];

      temp++;
   }

}
HybridAutomata::~HybridAutomata()
{
   cout << "~HybridAutomata called" << endl;
   for (int i = initState; i < exitState;i++)
   {
      for (int j =initState; j < exitState; j++)
      {
         if(conditions[i][j] != NULL)
            delete conditions[i][j];
      }
      if (states[i] != NULL)
         delete states[i];
   }
}
