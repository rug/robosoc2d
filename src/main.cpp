// (c) 2021 Ruggero Rossi
//#define _R2S_DEBUG
#include "simulator.h"
#include "simple_player.h"

using namespace std;
using namespace r2s;

#include <chrono>
//using namespace std::chrono;
#include <cstring>
#include <iostream>
#include "debug_print.h"

int main(int argc, char** argv)
{
  int nPlayers=4;
  int nPlayers1=(argc>1) ? stoi(argv[1]) : nPlayers;
  int nPlayers2=(argc>2) ? stoi(argv[2]) : nPlayers;
  unsigned int random_seed=(argc>3) ? stoul(argv[3]) : createChronoRandomSeed() ;

  cout << "Very Simplified 2D Robotic Soccer Simulator" << endl;
  std::unique_ptr<R2Simulator> simulator=  buildSimulator<SimplePlayer,SimplePlayer>(nPlayers1, nPlayers2, defaultTeam1Name, defaultTeam2Name, random_seed);
  
  char status_msg[256];
  DEBUG_OUT("\n random seed:");
  DEBUG_OUT(simulator->getRandomSeed());
  DEBUG_OUT("\n");
  while(simulator->stepIfPlaying()){
    //R2Environment env=simulator->getGameState().env;
    strcpy(status_msg, simulator->getStateString().c_str() );
    DEBUG_OUT(status_msg);
    DEBUG_OUT("\n");
  }
  strcpy(status_msg, simulator->getStateString().c_str() ); // to show also what happened in last tick
  DEBUG_OUT(status_msg);
  DEBUG_OUT("\n");
  //simulator->saveHistory();
}

/* 
to compile this without CMAKE or any other type of project:
create a new file named "robosoc2d.cpp" containing only the following three lines:

#include "simulator.cpp"
#include "simple_player.cpp"
#include "main.cpp"

and compile it with the following command:
 g++ robosoc2d.cpp -std=c++17 -lm 
*/