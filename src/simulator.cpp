// (c) 2021 Ruggero Rossi
// robosoc2d : a Very Simplified 2D Robotic Soccer Simulator
#ifdef _WIN32
  #include <numeric>
#else // __linux__ 
#endif

#include "simulator.h"
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

namespace r2s {

//sets angle between 0 and 2 PI
double fixAnglePositive(double angle){
  double angle2=remainder(angle, 2*M_PI);
  if(angle2< 0.0)
    angle2= 2*M_PI + angle2;
  return angle2;
}

//sets angle between + and - PI
double fixAngleTwoSides(double angle){
  double angle2=remainder(angle, 2*M_PI);
  //if(angle2< -M_PI) // it would never enter this condition because remainder returns the remaind to the number=quotient*divisor that is closer to dividend, so it will not be, in absolute terms, bigger than divisor/2  
  //  angle2= 2*M_PI + angle2;
  //if(angle2> M_PI)  // for the same reason it would never enter this condition as well. Different kind of "remainder" or "module" functions would need those conditions though.
  //  angle2= -2*M_PI + angle2;
  return angle2;
}

void R2Simulator::limitPlayersCloseToPitch(){
  for(int w=0; w<=1; w++)
    for(int n=0; n< env.teams[w].size(); n++){
      auto& p= env.teams[w][n];
      if(p.pos.x < pitch.border_left)
        p.pos.x = pitch.border_left;
      else if(p.pos.x > pitch.border_right)
        p.pos.x = pitch.border_right;

      if(p.pos.y < pitch.border_down)
        p.pos.y = pitch.border_down;
      else if(p.pos.y > pitch.border_up)
        p.pos.y = pitch.border_up;
    }
}

void R2Simulator::setBallThrowInPosition(){
  double borderY= (env.ball.pos.y > 0.0) ? pitch.y1 : pitch.y2;
  double intersectionX=0.0;
  Vec2 delta=env.ball.pos-oldEnv.ball.pos;
  if(fabs(delta.y)>R2BigEpsilon){
    double m=delta.x/delta.y;
    intersectionX=oldEnv.ball.pos.x + m*(borderY-oldEnv.ball.pos.y);
  }
  else{
    intersectionX=(oldEnv.ball.pos.x+env.ball.pos.x)/2;
  }
  env.ball.pos.x=intersectionX;
  env.ball.pos.y=borderY;
}

bool R2Simulator::isBallInGoal(const int team){ // team : in teams[team] own goal 
  if( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) // score in Team2 own goal
    return ( (env.ball.pos.x > pitch.x1) && (env.ball.pos.x < pitch.xGoal1) && (env.ball.pos.y > pitch.yGoal2) && (env.ball.pos.y < pitch.yGoal1) );
  else
    return ( (env.ball.pos.x > pitch.xGoal2) && (env.ball.pos.x < pitch.x2) && (env.ball.pos.y > pitch.yGoal2) && (env.ball.pos.y < pitch.yGoal1) );
}

bool R2Simulator::didBallIntersectGoalLine(const int team){
  if((team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed)){ // score in Team2 own goal
    if(env.ball.pos.x < pitch.x1)
      return false;
  }
  else{
    if(env.ball.pos.x > pitch.x2)
      return false;
  }

  double goalX= ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) )? pitch.x1 : pitch.x2;
  Vec2 delta=env.ball.pos-oldEnv.ball.pos;
  if(delta.x!=0.0){
    double m=delta.y/delta.x;
    double intersectionY=oldEnv.ball.pos.y + m*(goalX-oldEnv.ball.pos.x);
    if((intersectionY > pitch.yGoal2) &&(intersectionY < pitch.yGoal1)){
      return true;
    }
  }
  return false;
}

bool R2Simulator::isGoalScored(int team){ 
  if(didBallIntersectGoalLine(team)){
    return true;
  }
  // if it entered the goal passing through an external path, reposition the ball in an outside zone
  if(isBallInGoal(team)){
    double goalX= ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) ? pitch.x1 : pitch.x2;
    double epsilonOut= ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) ? R2Epsilon : -R2Epsilon;
    Vec2 delta=env.ball.pos-oldEnv.ball.pos;
    if(delta.x!=0.0){
      double m=delta.y/delta.x;
      double intersectionY=oldEnv.ball.pos.y + m*(goalX-oldEnv.ball.pos.x);
      env.ball.pos.x=goalX+epsilonOut;
      env.ball.pos.y=intersectionY;
    }
    env.ball.velocity.zero();
  }
  return false;
}

// we check the center of the body of the player
bool R2Simulator::isPlayerOut(int player, int team){
  auto& pos= env.teams[team][player].pos;
  return ( (pos.x < pitch.x2) || (pos.x > pitch.x1) || (pos.y < pitch.y2) || (pos.y > pitch.y1) );
}

// we check the center of the body of the player
bool R2Simulator::isPlayerInsideHisArea(const int player, const int team){
  auto& pos= env.teams[team][player].pos;
  if( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) )
    return ( (pos.x <= pitch.x1) && (pos.x >= pitch.areaRx) && (pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy) );
  else 
    return ( (pos.x >= pitch.x2) && (pos.x <= pitch.areaLx) && (pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy) );
}

// we check the center of the body of the player
bool R2Simulator::isPlayerInsideOpponentArea(const int player, const int team){
  auto& pos= env.teams[team][player].pos;
  if(! ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) )
    return ( (pos.x <= pitch.x1) && (pos.x >= pitch.areaRx) && (pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy) );
  else 
    return ( (pos.x >= pitch.x2) && (pos.x <= pitch.areaLx) && (pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy) );
}

// we check the full diameter of the body of the player
// team == 1: the player is of teams[1] and is checked against teams[0] area (left)
bool R2Simulator::isPlayerInsideOpponentAreaFullBody(Vec2 pos, const int team){
  if(! ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) ){
    bool insideCenterX=(pos.x <= pitch.x1) && (pos.x >= pitch.areaRx);
    bool insideCenterY=(pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy);
    if( insideCenterX && insideCenterY )
      return true;
    if(insideCenterX)
      return (( (pos.y-sett.playerRadius) <= pitch.areaUy) && ( (pos.y+sett.playerRadius) >= pitch.areaDy)) ;
    if(insideCenterY)
      return (( (pos.x-sett.playerRadius) <= pitch.x1) && ( (pos.x+sett.playerRadius) >= pitch.areaRx));
    // distance from upper or lower area corner
    Vec2 vert(pitch.areaRx, pitch.areaDy);
    if(pos.y>0.0){
      vert.y=pitch.areaUy;
    }
    return (pos.dist(vert) < sett.playerRadius);
  }
  else {
    bool insideCenterX=(pos.x >= pitch.x2) && (pos.x <= pitch.areaLx);
    bool insideCenterY=(pos.y <= pitch.areaUy) && (pos.y >= pitch.areaDy);
    if( insideCenterX && insideCenterY )
      return true;
    if(insideCenterX)
      return (( (pos.y-sett.playerRadius) <= pitch.areaUy) && ( (pos.y+sett.playerRadius) >= pitch.areaDy));
    if(insideCenterY)
      return (( (pos.x+sett.playerRadius) >= pitch.x2) && ( (pos.x-sett.playerRadius) <= pitch.areaLx));
    // distance from upper or lower area corner
    Vec2 vert(pitch.areaLx, pitch.areaDy);
    if(pos.y>0.0){
      vert.y=pitch.areaUy;
    }
    return (pos.dist(vert) < sett.playerRadius);
  }
  return false;
}

bool R2Simulator::isPlayerInsideOpponentAreaFullBody(const int player, const int team){
  return isPlayerInsideOpponentAreaFullBody( env.teams[team][player].pos, team);
}

void R2Simulator::limitBallSpeed(){
  double absSpeed=env.ball.absVelocity();
  if(absSpeed>sett.maxBallSpeed){
    double ratio=sett.maxBallSpeed/absSpeed;
    env.ball.velocity*=ratio;
  }
}

void R2Simulator::limitPlayerSpeed(R2PlayerInfo& p){
  double absSpeed=p.absVelocity();
  if(absSpeed>sett.maxPlayerSpeed)
  {
    double ratio=sett.maxPlayerSpeed/absSpeed;
    p.velocity*=ratio;
  }
}

void R2Simulator::limitSpeed(){
  limitBallSpeed();
  for(int i=0; i<2; i++)
    for(auto& p : env.teams[i])
        limitPlayerSpeed(p);
}

void R2Simulator::decayPlayerSpeed(R2PlayerInfo& p){
  if((p.velocity.x==0.0)&&(p.velocity.y==0.0))
    return;

  //finding and fix 2D gymbal lock
  double velAngle= atan2(p.velocity.y, p.velocity.x);
  double velAngleBis=fixAnglePositive(velAngle);
  double playerDir=p.direction;
  double diff=playerDir-velAngleBis;
  if(diff>M_PI)
    playerDir-=2*M_PI;
  else if(diff<-M_PI)
    velAngleBis-=2*M_PI;

  double len=p.velocity.len();
  double newAngle= velAngleBis*(1.0 - sett.playerVelocityDirectionMix) + playerDir*sett.playerVelocityDirectionMix;
  Vec2 newVelocity(newAngle);
  p.velocity=newVelocity*len*sett.playerVelocityDecay;
}

void R2Simulator::decaySpeed(){
  env.ball.velocity *= sett.ballVelocityDecay;
  
  for(auto& p : env.teams[0])
    decayPlayerSpeed(p);
  for(auto& p : env.teams[1])
    decayPlayerSpeed(p);
}

void R2Simulator::setBallCatchedPosition(){
    auto& goalkeeper=env.teams[env.ballCatchedTeam2][0];
    double d=sett.playerRadius -sett.ballRadius -sett.afterCatchDistance;
    env.ball.pos.x=goalkeeper.pos.x+cos(goalkeeper.direction)*d;
    env.ball.pos.y=goalkeeper.pos.y+sin(goalkeeper.direction)*d;
    env.ball.velocity=goalkeeper.velocity;
}
void R2Simulator::setBallReleasedPosition(){
    auto& goalkeeper=env.teams[env.ballCatchedTeam2][0];
    double d=sett.playerRadius +sett.ballRadius +sett.afterCatchDistance;
    env.ball.pos.x=goalkeeper.pos.x+cos(goalkeeper.direction)*d;
    env.ball.pos.y=goalkeeper.pos.y+sin(goalkeeper.direction)*d;
    env.ball.velocity=goalkeeper.velocity;
}

// team is the team to be put far from ball
void R2Simulator::putPlayersFarFromBall(int team, double minDist){
  for(auto& p: env.teams[team])
  {
    auto [dist, d]=p.dist(env.ball); 
    if(dist < minDist){
      if(dist<R2Epsilon) {
        double angle=uniformDist(rng)*2*M_PI;
        double sx=cos(angle)*minDist;
        double sy=sin(angle)*minDist;
          p.pos.x-=sx;
          p.pos.y-=sy;
      }
      else {
        double ratio=minDist/dist;
        double diff=ratio-1.0;
        if(diff>0.0){
          p.pos.x-=d.x*diff;
          p.pos.y-=d.y*diff;
        }
      }
    }
  }
}

//returns a new position that avoids, if possible, to intersect the other players that already acted.
Vec2 R2Simulator::avoidOtherPlayersPosition(Vec2 pos, int team, int player){  
  bool collisions=true;
  int count=0;
  while(collisions && (count <MaxCollisionLoop)){
    collisions=false;
    for(int w=0; w<=1; w++)
      for(int n=0; n< env.teams[w].size(); n++){
        if((w==team)&&(n==player))
          continue;
        auto other=env.teams[w][n];
        if(other.acted){
          Vec2 delta=other.pos-pos;
          double dist=delta.len();
          if(dist<R2Epsilon) {
            collisions=true;
            double angle=uniformDist(rng)*2*M_PI;
            double sx=cos(angle)*sett.playerRadius*2;
            double sy=sin(angle)*sett.playerRadius*2;
            pos.x+=sx;
            pos.y+=sy;
          }
          else {
            double ratio=sett.playerRadius*2/dist;
            if(ratio>1.0){
              collisions=true;
              double toAdd= ratio-1.0;
              pos.x+=delta.x*toAdd;
              pos.y+=delta.y*toAdd;
            }
          }
        }
      }

    count ++;
  }
  return pos;
}

void R2Simulator::actionMove(const R2Action& action, int team, int player){
  auto& p= env.teams[team][player];
  Vec2 pos(action.data[0],action.data[1]);

  p.direction=fixAnglePositive(action.data[2]);
  p.velocity.zero();

  //stay away from other people if they already moved
  pos=avoidOtherPlayersPosition(pos, team, player);
  p.pos=pos;
}

void R2Simulator::actionMoveKickoff(const R2Action& action, int team, int player){
  auto& p= env.teams[team][player];
  Vec2 pos(action.data[0],action.data[1]);

  p.direction=fixAnglePositive(action.data[2]);
  p.velocity.zero();

  //stay in your half pitch
  if(! ( (team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed) ) ){
    if (pos.x > -sett.playerRadius)
        pos.x=-sett.playerRadius;
  }
  else{
    if (pos.x < sett.playerRadius)
        pos.x=sett.playerRadius;
  }

  //stay far from the center circle
  double dueDistance=sett.centerRadius+sett.playerRadius;
  if( (env.state==R2State::Kickoff1) && team ){
      double dist=pos.len();
      if(dist<=R2Epsilon){
        pos.x=dueDistance;
        pos.y=uniformDist(rng);
      }
      else if (dist < dueDistance)
      {
        double ratio=dueDistance/dist;
        pos.x*=ratio;
        pos.y*=ratio;
      }
  }
  else if( (env.state==R2State::Kickoff2) && (!team) ){
      double dist=pos.len();
      if(dist<=R2Epsilon){
        pos.x=-dueDistance;
        pos.y=uniformDist(rng);
      }
      else if (dist < dueDistance)
      {
        double ratio=dueDistance/dist;
        pos.x*=ratio;
        pos.y*=ratio;
      }
  }

  //stay away from other people if they already moved
  pos=avoidOtherPlayersPosition(pos, team, player);
  p.pos=pos;;
}

void R2Simulator::actionMoveGoalkick(const R2Action& action, int team, int player){
  auto& p= env.teams[team][player];
  Vec2 pos(action.data[0],action.data[1]);
  double dir=fixAnglePositive(action.data[2]);

  Vec2 displace=pos-p.pos;
  //player of the kicking team: taking note of the longest movement
  if( ( ((env.state==R2State::Goalkick1up)||(env.state==R2State::Goalkick1down)) && (!team) ) || 
      ( ((env.state==R2State::Goalkick2up)||(env.state==R2State::Goalkick2down)) && team ) ){
    double movement=displace.len();
    if(movement>env.startingTeamMaxRange)
      env.startingTeamMaxRange=movement;
  }
  else { // check the max movement you can do
    if(displace.len()>env.startingTeamMaxRange){
      displace.resize(env.startingTeamMaxRange);
      pos=p.pos+displace;
    }
  }

  // if not of kicker team stay away fom the area
  if( ((env.state==R2State::Goalkick1up)||(env.state==R2State::Goalkick1down)) && team ){  // stay away from teams[0] area (left area)
    if(isPlayerInsideOpponentAreaFullBody(pos, true))
        pos.x=pitch.areaLx+sett.playerRadius;
  }
  else if( ((env.state==R2State::Goalkick2up)||(env.state==R2State::Goalkick2down)) && (!team) ) { // stay away from teams[1] area (right area)
    if(isPlayerInsideOpponentAreaFullBody(pos, false))
        pos.x=pitch.areaRx-sett.playerRadius;
  }

  //stay away from other people if they already moved
  pos=avoidOtherPlayersPosition(pos, team, player);

  p.velocity.zero();
  p.pos=pos;
  p.direction=dir; 
}

void R2Simulator::actionMoveThrowinCorner(const R2Action& action, int team, int player, double distanceToBall){
  auto& p= env.teams[team][player];
  Vec2 pos(action.data[0],action.data[1]);
  double dir=fixAnglePositive(action.data[2]);

  Vec2 displace=pos-p.pos;
  //player of the kicking team: taking note of the longest movement
  if( ( ((env.state==R2State::Throwin1)||(env.state==R2State::Corner1up)||(env.state==R2State::Corner1down)) && (!team) ) ||
      ( ((env.state==R2State::Throwin2)||(env.state==R2State::Corner2up)||(env.state==R2State::Corner2down)) && team ) ){
    double movement=displace.len();
    if(movement>env.startingTeamMaxRange)
      env.startingTeamMaxRange=movement;
  }
  else { // check the max movement you can do
    if(displace.len()>env.startingTeamMaxRange){
      displace.resize(env.startingTeamMaxRange);
      pos=p.pos+displace;
    }
  }

  // if not of kicker team stay away fom the ball
  if( ( ((env.state==R2State::Throwin1)||(env.state==R2State::Corner1up)||(env.state==R2State::Corner1down)) && team ) || 
      ( ((env.state==R2State::Throwin2)||(env.state==R2State::Corner2up)||(env.state==R2State::Corner2down)) && (!team) ) ){ 
    double d=pos.dist(env.ball.pos);
    while(d<distanceToBall){
      Vec2 line=pos-env.ball.pos;
      line.resize(distanceToBall);
      pos=env.ball.pos+line;
    }
  }

  //stay away from other people if they already moved
  pos=avoidOtherPlayersPosition(pos, team, player);

  p.velocity.zero();
  p.pos=pos;
  p.direction=dir; 
}

void R2Simulator::actionMoveThrowin(const R2Action& action, int team, int player){
  actionMoveThrowinCorner(action, team, player, sett.throwinMinDistance);
}

void R2Simulator::actionMoveCorner(const R2Action& action, int team, int player){
  actionMoveThrowinCorner(action, team, player, sett.cornerMinDistance);
}

void R2Simulator::actionDash(const R2Action& action, int team, int player){
  auto& p= env.teams[team][player];
  // limit power
  double power=action.data[1];
  double reverse=0.0;
  if(power < 0.0){
    power= -power;
    reverse=1.0;
  }
  power+=sett.playerRandomNoise*(normalDist(rng)-0.5)*action.data[1];  //add random
  if(power <0.0)
    power=0.0;

  if(power >=MaxDashPower)
    power=MaxDashPower;
  
  double angle=action.data[0] + reverse*M_PI + sett.playerDirectionNoise*(normalDist(rng)-0.5);

  p.velocity.x+=cos(angle)*power;
  p.velocity.y+=sin(angle)*power;
  p.direction=fixAnglePositive(angle);
}

void R2Simulator::actionKick(const R2Action& action, int team, int player){
  auto& p= env.teams[team][player];
  bool canKick=true;
  bool catchedKicking=false;

  if(env.ballCatched){
    if ((player!=0) || ( int(env.ballCatchedTeam2) != team)){
      canKick=false;
    }
    else{
      catchedKicking=true;
      env.ballCatched=0;
    }
  } 

  auto [dist, d]=p.dist(env.ball); 
  // is ball reachable ?
  if(dist>sett.kickableDistance)
    canKick=false;

  if(! sett.simplified){
    // is ball in front of player?
    if(!catchedKicking)
      if(canKick && (!isAnyTeamKicking())){
        double cosinusPlayerBall= (d.x*cos(p.direction) + d.y*sin(p.direction))/dist;
        if(cosinusPlayerBall < cosKickableAngle)
          canKick=false;
      }
  
    // is ball inside some player?
    if(!catchedKicking)
      if(canKick)
        for(int w=0; w<=1; w++)
          for(int n=0; n< env.teams[w].size(); n++){
            auto pl=env.teams[w][n];
            double d=(pl.pos-env.ball.pos).len();
            if(d<sett.playerRadius)  // well inside the player
              canKick=false;
          }
  }
  
  
  // limit power
  double reverse=0.0;
  double power=action.data[1];
  if(power < 0.0){
    power= -power;
    reverse=1.0;
  }
  power+=sett.playerRandomNoise*(normalDist(rng)-0.5)*action.data[1];  //add random
  if(power <0.0)
    power=0.0;

  double angle=action.data[0] + reverse*M_PI;
  double kickAngle=fixAnglePositive(angle);
  if(! sett.simplified){
    if(canKick && (!isAnyTeamKicking())){
      if( fabs( remainder( kickAngle-p.direction , 2*M_PI ) ) > sett.kickableDirectionAngle) // if angle between player direction and kick direction > kickableDirectionAngle or < -kickableDirectionAngle
        canKick=false;
    }
  }


  if(canKick && ballAlreadyKicked){
    if(! sett.simplified){
      if(uniformDist(rng)>ContemporaryKickProbability)
        canKick=false;
    }
    else{
      canKick=false;
    }
  }
  
  kickAngle+=sett.playerDirectionNoise*(normalDist(rng)-0.5);
  kickAngle=fixAnglePositive(kickAngle);

  p.direction=kickAngle;

  if(catchedKicking){ //move the ball to the first point outside the player
      setBallReleasedPosition();
  }
  else if(sett.simplified && canKick && (!isAnyTeamKicking()) ){ //if ball behind or too lateral, put it in front
    auto [dist, delta]=p.dist(env.ball); 
    double cosinusPlayerBall= (delta.x*cos(p.direction) + delta.y*sin(p.direction))/dist;
    if(cosinusPlayerBall < 0.707){ // less than 45 degrees
      double d=sett.playerRadius +sett.ballRadius +sett.afterCatchDistance;
      //std::cout << "old ball position: x=" << env.ball.pos.x << " y=" << env.ball.pos.y << "\n";
      env.ball.pos.x=p.pos.x+cos(kickAngle)*d;
      env.ball.pos.y=p.pos.y+sin(kickAngle)*d;
      //std::cout << "new ball position: x=" << env.ball.pos.x << " y=" << env.ball.pos.y << "\n";
      env.ball.velocity.x=0;
      env.ball.velocity.y=0;
    }
  }

  if(canKick){
    
    env.lastTouchedTeam2=team;
    if(power >= MaxKickPower)
      power=MaxKickPower;

    Vec2 kickDirection( kickAngle );

    env.ball.velocity.x = 0.0;
    env.ball.velocity.y = 0.0;

    env.ball.velocity += kickDirection*power;
    ballAlreadyKicked=true;
    
  }
}

void R2Simulator::actionCatch(const R2Action& action, int team, int player){
  if( (player >0) || (!isPlayerInsideHisArea(player, team)) )// only goalkeeper, inside his area
    return;

  if( (player>0)  || //only goalkeeper
      (!isPlayerInsideHisArea(player, team)) ) //inside his area
    return;

  if(env.ballCatched)
    return;

  // check if ball is in front and reachable
  auto& p= env.teams[team][player];

  auto [dist, d]=p.dist(env.ball); 
  // is ball reachable
  if(dist>(sett.catchableDistance) )
    return;

  // is ball in front of player?
  double cosinusPlayerBall= (d.x*cos(p.direction) + d.y*sin(p.direction))/dist;
  if(cosinusPlayerBall < cosCatchableAngle)  // angle > 90 or < -90 between player direction and ball direction
    return;

  // check catch probability
  if(uniformDist(rng) <=sett.catchProbability)
  {
    env.ball.velocity.x=0.0;
    env.ball.velocity.y=0.0;
    setBallCatchedPosition();
    env.lastTouchedTeam2=team;
    env.ballCatchedTeam2=team;
    env.ballCatched=sett.catchHoldingTicks;
  }
}

// is a team prepare for kicking from stopped game ?
bool R2Simulator::isAnyTeamPreparingKicking(){
  if(notStarterStates.count(env.state))
    return false;
  return true;
}

// is a team kicking from stopped game ?
bool R2Simulator::isAnyTeamKicking(){
  if((env.state==R2State::Play) && (notStarterStates.count(oldEnv.state)==0))
    return true;
  return false;
}

bool R2Simulator::isTeam2Kicking(R2State theState){
  if(team2StarterStates.count(theState))
    return true;
  return false;
}

void R2Simulator::resetPlayersActed(){
  //reset acting info
  for(int w=0; w<=1; w++)
    for(int n=0; n< env.teams[w].size(); n++){
      env.teams[w][n].acted=false;
    }
}

void R2Simulator::playersAct(){
  if(isAnyTeamPreparingKicking() ){ // preparing kicking: the kicking team acts first, with the closest player acting first.
    int kickingTeam= int(isTeam2Kicking(env.state));
    int sizeKickingTeam=teams[kickingTeam].size();
    // let's find the closest to the ball
    int closest=0;
    if(sizeKickingTeam){
      double minDist=env.teams[kickingTeam][0].pos.dist(env.ball.pos);
      for(int n=1; n< env.teams[kickingTeam].size(); n++){
        double distance=env.teams[kickingTeam][n].pos.dist(env.ball.pos);
        if(distance < minDist){
          minDist=distance;
          closest=n;
        }
      }

      //let's have the closest player acting first
      R2Action action = teams[kickingTeam][closest]->step( getGameState() ); 
      env.teams[kickingTeam][closest].acted=true;
      processStep(action, kickingTeam, closest);

      //then all of his own team except him
      for(int n=0; n< sizeKickingTeam; n++){
        if(n!=closest){
          action = teams[kickingTeam][n]->step( getGameState() ); // updated game state for each player
          env.teams[kickingTeam][n].acted=true;
          processStep(action, kickingTeam, n);
        }
      }
    }

    //then all other team
    int team= 1-kickingTeam;
    for(int n=0; n< env.teams[team].size(); n++){
      R2Action action = teams[team][n]->step( getGameState() ); // updated game state for each player
      env.teams[team][n].acted=true;
      processStep(action, team, n);
    }
  }
  else if(isAnyTeamKicking() ){ //kicking right now - only the kicking team's player that's closest to the ball starts first
    int kickingTeam= int(isTeam2Kicking(oldEnv.state));

    // let's find the closest to the ball
    int closest=0;
    if(teams[kickingTeam].size()){
      double minDist=env.teams[kickingTeam][0].pos.dist(env.ball.pos);
      for(int n=1; n< env.teams[kickingTeam].size(); n++){
        double distance=env.teams[kickingTeam][n].pos.dist(env.ball.pos);
        if(distance < minDist){
          minDist=distance;
          closest=n;
        }
      }

      //let's have the closest player of the kicking team acting first
      R2Action action = teams[kickingTeam][closest]->step( getGameState() );
      env.teams[kickingTeam][closest].acted=true;
      processStep(action, kickingTeam, closest);
    }

    //now all the rest, shuffled
    shuffle(begin(shuffledPlayers), end(shuffledPlayers), rng);
    for(int i: shuffledPlayers){
      int whichTeam=0;
      
      if(int index_team2= i - teams[0].size(); index_team2>=0){
          whichTeam = 1;
        i=index_team2;
      }
      if((i!=closest)||(whichTeam !=kickingTeam)){
        R2Action action = teams[whichTeam][i]->step( getGameState() ); // updated game state for each player
        env.teams[whichTeam][i].acted=true;
        processStep(action, whichTeam, i);
      }
    }
  }
  else{  // if not right after a stop-game begin, the player order is shuffled
    shuffle(begin(shuffledPlayers), end(shuffledPlayers), rng);
    auto gameState = getGameState();
    for(int i: shuffledPlayers){
      int whichTeam = 0;
      
      if(int index_team2= i - teams[0].size(); index_team2>=0){
          whichTeam = 1;
        i=index_team2;
      }
      R2Action action = teams[whichTeam][i]->step( gameState ); // same game state for each player
      env.teams[whichTeam][i].acted=true;
      processStep(action, whichTeam, i);
    }
  }
}

void R2Simulator::step(){
  history.envs[env.tick]=env;
  processedActions=0;

  resetPlayersActed();
  preState();
  playersAct();
  limitSpeed();
  limitPlayersCloseToPitch();
  checkState();
  decaySpeed();
  env.tick += 1;
};

bool R2Simulator::checkBallOut(){

  auto doBallLeftUp=[&](){
    if(env.lastTouchedTeam2){
      if(!env.halftimePassed)
        env.state=R2State::Goalkick1up;
      else
        env.state=R2State::Corner1up;
    }
    else{
      if(!env.halftimePassed)
        env.state=R2State::Corner2up;
      else
        env.state=R2State::Goalkick2up;      
    }
  };

  auto doBallLeftDown=[&](){
    if(env.lastTouchedTeam2){
      if(!env.halftimePassed)
        env.state=R2State::Goalkick1down;
      else
        env.state=R2State::Corner1down;
    }
    else{
      if(!env.halftimePassed)
        env.state=R2State::Corner2down;
      else
        env.state=R2State::Goalkick2down;
    }
  };

  auto doBallRightUp=[&](){
    if(env.lastTouchedTeam2){
      if(!env.halftimePassed)
        env.state=R2State::Corner1up;
      else 
        env.state=R2State::Goalkick1up;
    }
    else{
      if(!env.halftimePassed)
        env.state=R2State::Goalkick2up;
      else
        env.state=R2State::Corner2up;
    }
  };

  auto doBallRightDown=[&](){
    if(env.lastTouchedTeam2){
      if(!env.halftimePassed)
        env.state=R2State::Corner1down;
      else
        env.state=R2State::Goalkick1down;
    }
    else{
      if(!env.halftimePassed)
        env.state=R2State::Goalkick2down;
      else
        env.state=R2State::Corner2down;
    }
  };

  auto doBallLeft=[&](){
    if (env.ball.pos.y>0.0)
      doBallLeftUp();
    else
      doBallLeftDown();
  };

  auto doBallRight=[&](){
    if (env.ball.pos.y>0.0)
      doBallRightUp();
    else
      doBallRightDown();
  };

  Vec2 d=env.ball.pos - oldEnv.ball.pos;
  if(d.y==0.0){
    if(env.ball.pos.x < pitch.x2){
      doBallLeft();
      return true;
    }
    else if(env.ball.pos.x > pitch.x1){
      doBallRight();
      return true;
    }
    return false;
  }
  double ratio=d.x/d.y;

  if(isBallOutUp()){
    double du=pitch.y1-oldEnv.ball.pos.y;
    double hx=ratio*du+oldEnv.ball.pos.x;

    auto doBallUp=[&](){
      if(env.lastTouchedTeam2)
        env.state=R2State::Throwin1;
      else
        env.state=R2State::Throwin2;
    };

    if(isBallOutLeft()){
      if(hx<pitch.x2){ 
        doBallLeftUp();
      }
      else{ 
        doBallUp();
      }
    }
    else if(isBallOutRight()){
      if(hx>pitch.x1){  
        doBallRightUp();
      }
      else{ 
        doBallUp();
      }
    }
    else{ 
      doBallUp();
    }
    return true;
  }
  else if(isBallOutDown()){
    double du=pitch.y2-oldEnv.ball.pos.y;
    double hx=ratio*du+oldEnv.ball.pos.x;

    auto doBallDown=[&](){
      if(env.lastTouchedTeam2)
        env.state=R2State::Throwin1;
      else 
        env.state=R2State::Throwin2;
    };
    
    if(isBallOutLeft()){
      if(hx<pitch.x2){  
        doBallLeftDown();
      }
      else{ 
        doBallDown();
      }
    }
    else if(isBallOutRight()){
      if(hx>pitch.x1){  
        doBallRightDown();
      }
      else{ 
        doBallDown();
      }
    }
    else{ 
      doBallDown();
    }
    return true;
  }
  else if(isBallOutLeft()){
    doBallLeft();
    return true;
  }
  else if(isBallOutRight()){
    doBallRight();
    return true;
  }
  return false;
}

/**
@param s1 and s2 the initial and final points of the segment
@param c1 the center of the circle
@param r the radius of the circle
*/
// returns:
// int: number of intersections (0, 1, 2)
// double: t for first intersection (if any)
// double: t for second intersection (if existent)
///////////////////////////////////////////
// how it works:
// circle: (x - c1.x)^2 + (y - c1.y)^2 = r^2
// segment: x(t)= (s1.x - s2.x)*t + s1.x
//          y(t)= (s1.y - s2.y)*t + s1.y
// with 0 <= t <= 1
// putting together as in a system:
//  ((s1.x - s2.x)*t + s1.x - c1.x)^2 + ( (s1.y - s2.y)*t + s1.y - c1.y)^2 = r^2
// solving for t as a quadratic equation a*t^2 + b*t + c = 0
// with:
// a= (s1.x - s2.x)^2 - (s1.y - s2.y)^2
// b= 2*(s1.x - s2.x)*(s1.x - c.x) + 2*(s1.y - s2.y)*(s1.y - 1.y)
// c= (s1.x - c1.x)^2 + (s1.y - c1.y)^2 - r^2
// results:
// delta= b^2 - 4*a*c  // if <0 no intersection, if ==0 one tangent point, if >0 two intersections
// t= (b +- sqrt(delta)) / (2*a)
std::tuple<int, double, double> intersectionSegmentCircle(Vec2 s1, Vec2 s2, Vec2 c1, double r){
  Vec2 d= s1 - s2;
  Vec2 l= s1 - c1;
  double a= d.x*d.x - d.y*d.y;
  double b= 2.0*d.x*l.x + 2*d.y*l.y;
  double c= l.x*l.x + l.y*l.y - r*r;
  double delta= b*b - 4.0*a*c;
  if(delta < 0.0){
    return std::tuple<int, double, double>  { 0, 0.0, 0.0 };
  }
  else if(delta == 0.0){
    return std::tuple<int, double, double>  { 1, b/(2.0*a), 0.0 };
  }
  else{
    double deltaRoot= sqrt(delta);
    return std::tuple<int, double, double>  { 2, (b-deltaRoot)/(2.0*a), (b+deltaRoot)/(2.0*a) };
  }
}

// The ball is a circle moving by a rectilinear uniform motion during the tick (acceleration/deceleration changes the velocity only between a tick and the next) and it may be
// intersecting the player that is another circle moving by a rectilinear uniform motion.
// From a geometrical point of view, if considering a reference frame with respect to a circle (e.g. wrt the player), hence considering that reference circle as still,
// this is the same as having the other circle (e.g. the ball) moving by a rectilinear uniform motion whose uniform velocity is the resultant of the velocity of the moving circle minus
// the velocity of the "now-still" circle. 
// That in turn is equivalent to the intersection of a point moving by the same rectilinear trajectory (hence a line) intersecting a still circle that has the radius equal to 
// the sum of the radiuses of the two circles.
// THIS MEANS THAT WE NEED THE POSITION OF ALL PLAYER AND BALL OF THE PAST TICK.
// In this way if there is an intersection, the position of intersection will determine
// the position of the center of the ball when it collides with the player (before entering "inside" the player).
// (there may be zero intersections, or one if the line is tangent to the circle, or two if proper intersection).
// With zero intersections there is nothing to do.
// With one intersection I suggest to do nothing, there is not really an impact (one may want to calculate some friction effect though)
// With two intersection we need to find the closer in time.
// To transform this in the world where both circles are moving, its enough to consider the proportion of the trajectory line on which the intersection happened,
// then that is the proportion in which the uniform motion of the player and the ball collided.
// To simulate rightly what happens with all the players we should calculate the collision points of the ball with all the players, then
// take the collision that happened earlier: that is the only collision that actually happened.
// From that, calculate a new velocity/trajectory for the ball, considering only the remaining proportion of tick.
// Do the same, considering all the possible intersections (avoiding last intersected player) and go on until there is not an intersection anymore or until
// the maximum number of collision has ended.
// returns:
// bool: if intersection happened
// double: t of intersection
std::tuple<bool, double> R2Simulator::findObjectsCollision(R2ObjectInfo& obj1, R2ObjectInfo&obj2, double radius, double partialT)
{
  Vec2 s2= obj1.pos + (obj1.velocity - obj2.velocity)*(1.0 - partialT);
  auto [n, t1, t2]=intersectionSegmentCircle(obj1.pos, s2, obj2.pos, radius);
  if(n==2){ // if tangent, no collision really happened
    double t= (t1>=0.0) ? t1 : t2;  // we want the first intersection, unless it is less than 0.0 (that means there is not an actual intersection) in which case we check the second one(that is always greater than the first)
    if( (t>=R2Epsilon) && (t<=1.0) )
      return std::tuple<bool, double> {true, t} ;
  }
  return std::tuple<bool, double> {false, 0.0} ;
}

// returns:
// bool: if intersection happened
// double: t of intersection
std::tuple<bool, double> R2Simulator::findBallPlayerCollision(int team, int player, double partialT)
{
  return findObjectsCollision(env.ball, env.teams[team][player], sett.playerRadius+sett.ballRadius, partialT);
}

std::tuple<bool, double> R2Simulator::findPlayerPlayerCollision(int team1, int player1, int team2, int player2, double partialT){
    return findObjectsCollision(env.teams[team1][player1], env.teams[team2][player2], sett.playerRadius+sett.playerRadius, partialT);
}

std::tuple<bool, double> R2Simulator::findPoleObjectCollision(R2ObjectInfo& obj1, Vec2 pole, double radius, double partialT)
{
  Vec2 s2= obj1.pos + obj1.velocity*(1.0 - partialT);
  auto [n, t1, t2]=intersectionSegmentCircle(obj1.pos, s2, pole, radius);
  if(n==2){ // if tangent, no collision really happened
    double t= (t1>=0.0) ? t1 : t2;  // we want the first intersection, unless it is less than 0.0 (that means there is not an actual intersection) in which case we check the second one(that is always greater than the first)
    if( (t>=R2Epsilon) && (t<=1.0) )
      return std::tuple<bool, double> {true, t} ;
  }
  return std::tuple<bool, double> {false, 0.0} ;
}

// returns:
// bool: if intersection happened
// double: t of intersection
std::tuple<bool, double> R2Simulator::findPolePlayerCollision(int team, int player, Vec2 pole, double partialT){
  return findPoleObjectCollision(env.teams[team][player], pole, sett.playerRadius+sett.poleRadius, partialT);
}

std::tuple<bool, double> R2Simulator::findPoleBallCollision(Vec2 pole, double partialT){
  return findPoleObjectCollision(env.ball, pole, sett.ballRadius+sett.poleRadius, partialT);
}

R2PoleBallCollision R2Simulator::findFirstPoleBallCollision(double partialT){
  R2PoleBallCollision collision(false, 1.1, 0);
  for(int i=0; i<4; i++){
    auto[found, t]= findPoleBallCollision(pitch.poles[i], partialT);
    if(found) { // no need to do  && (t<collision.t) - only one collision possible
      collision.collision=true;
      collision.pole=i;
      collision.t=t;
      break; // can collide against only a pole at once
    }
  }
  return collision;
}

std::vector<R2PolePlayerCollision> R2Simulator::findFirstPolePlayersCollisions(double partialT){
  std::vector<R2PolePlayerCollision> collisions;
  double earlierT=1.1;
  for(int w=0; w<=1; w++)
    for(int n=0; n< env.teams[w].size(); n++){
      for(int i=0; i<4; i++){
        auto[found, t]= findPolePlayerCollision(w, n, pitch.poles[i], partialT);
        if(found){ 
          if(t<earlierT){
            collisions.clear();
          }
          if(t<=earlierT){
            earlierT=t;
            R2PolePlayerCollision collision(t, n, w, i);
            collisions.push_back(collision);
          }
          break;  // can collide against only a pole at once
        }
      }
    }
  return collisions;
}

std::vector<R2BallPlayerCollision> R2Simulator::findFirstBallPlayersCollisions(double partialT, std::vector<bool>& ballPlayerBlacklist){
  std::vector<R2BallPlayerCollision>  collisions;
  double earlierT=1.1;
  int t1size=env.teams[0].size();
  for(int w=0; w<=1; w++)
    for(int n=0; n< env.teams[w].size(); n++){
      if(ballPlayerBlacklist[t1size*w+n])
        continue;
      auto[found, t]= findBallPlayerCollision(w, n, partialT);
      if(found){ 
        if(t<earlierT){
          collisions.clear();
        }
        if(t<=earlierT){
          earlierT=t;
          R2BallPlayerCollision collision(t, n, w);
          collisions.push_back(collision);
        }
      }

    }
  return collisions;
}

std::vector<R2PlayerPlayerCollision> R2Simulator::findFirstPlayerPlayersCollisions(double partialT, std::vector<int>& playerPlayerCollisions){
  std::vector<R2PlayerPlayerCollision> collisions;
  int t1size=env.teams[0].size();
  double earlierT=1.1;
  for(int w1=0; w1<=1; w1++){
    int l1= env.teams[w1].size();
    for(int w2=0; w2<=1; w2++){
      if( w1 && (!w2) ) // do not check twice the collision betwenn the two teams
        continue;
      int l2= env.teams[w2].size();
        for(int n1=0; n1<l1; n1++){
          int start2 = (w1 == w2) ? (n1+1) : 0 ;
          for(int n2=start2; n2<l2; n2++){
              if(playerPlayerCollisions[w1*t1size+n1]!= (w2*t1size+n2) ){ // only if not just prior collision
                auto[found, t]= findPlayerPlayerCollision(w1, n1, w2, n2, partialT);
                if(found){ 
                  if(t<earlierT){
                    collisions.clear();
                  }
                  if(t<=earlierT){
                    earlierT=t;
                    R2PlayerPlayerCollision collision(t, n1, w1, n2, w2);
                    collisions.push_back(collision);
                  }
                }
              }
          }
        }
    }
  }
  return collisions;
}

// updates motion up to t
void R2Simulator::updateMotion(double t){
  env.ball.pos+=env.ball.velocity*t;
  for(int w=0; w<=1; w++)
    for(auto& p: env.teams[w]){
      p.pos+=p.velocity*t;
    }
} 

void R2Simulator::addBallNoise(){
  Vec2 noise=  Vec2((normalDist(rng)-0.5)*fabs(env.ball.velocity.x), (normalDist(rng)-0.5)*fabs(env.ball.velocity.y))*sett.playerRandomNoise;
  Vec2 newPos= env.ball.pos + noise;

  for(int w=0; w<=1; w++)
    for(int n=0; n< env.teams[w].size(); n++){
      auto& p= env.teams[w][n];
      if( (newPos.x<=(p.pos.x+sett.playerRadius+sett.ballRadius)) &&
          (newPos.x>=(p.pos.x-sett.playerRadius-sett.ballRadius)) &&
          (newPos.y<=(p.pos.y+sett.playerRadius+sett.ballRadius)) &&
          (newPos.y>=(p.pos.y+sett.playerRadius+sett.ballRadius)) ){
        return;
      }
    }
  
  env.ball.pos=newPos;
}

bool R2Simulator::checkGoalOrBallOut(){
  if (isGoalScored(false)) {
          env.score2 += 1;
          env.state = R2State::Goal2;
          env.ball.velocity.zero();
          return true;
  }
  else if (isGoalScored(true)) {
          env.score1 += 1;
          env.state = R2State::Goal1;
          env.ball.velocity.zero();
          return true;
  }
  
  return checkBallOut();
}

void R2Simulator::manageCollisions(){
  double partialT= 0.0;
  bool collisions=true;
  int count=0;
  double addT=0.0;

  int t1size=env.teams[0].size();
  int t2size=env.teams[1].size();

  std::vector<int> playerPlayerCollisions(t1size+t2size, -1);
  std::vector<bool>  ballPlayerBlacklist(t1size+t2size, false);

  int ballPlayersColls[MaxCollisionInsideTickLoop]; 
  int ballPlayersCollsTeam[MaxCollisionInsideTickLoop]; 
  int howManyBallPlayersColls=0;
  while(collisions && (count <MaxCollisionInsideTickLoop) && (partialT<1.0) ){
    collisions=false;
    std::vector<R2CollisionTime> earlierCollisionsTypes;

    R2PoleBallCollision newPoleBallColl(false, 0.0, 0);
    if(!env.ballCatched){
      newPoleBallColl=findFirstPoleBallCollision(partialT);
      if((newPoleBallColl.collision)&&(env.ball.velocity.len()>0.0)){
        collisions|=newPoleBallColl.collision;

        if(earlierCollisionsTypes.size()>0){
          double earlierT=earlierCollisionsTypes[0].t;
          if(newPoleBallColl.t<earlierT){
            earlierCollisionsTypes.clear();
          }
          if(newPoleBallColl.t<=earlierT){
            R2CollisionTime co(newPoleBallColl.t, R2CollisionType::PoleBall);
            earlierCollisionsTypes.push_back(co);
          }
        }
        else{
          R2CollisionTime co(newPoleBallColl.t, R2CollisionType::PoleBall);
          earlierCollisionsTypes.push_back(co);
        }
      }
    }

    std::vector<R2PolePlayerCollision> newPolePlayersColls= findFirstPolePlayersCollisions(partialT);
    bool collPolePlayers=(newPolePlayersColls.size()>0);
    collisions|=collPolePlayers;
    double kPolePlayers=0.0;
    if(collPolePlayers){
     kPolePlayers=newPolePlayersColls[0].t;
    }
    if(collPolePlayers){
      if(earlierCollisionsTypes.size()>0){
        double earlierT=earlierCollisionsTypes[0].t;
        if(kPolePlayers<earlierT){
          earlierCollisionsTypes.clear();
        }
        if(kPolePlayers<=earlierT){
          R2CollisionTime co(kPolePlayers, R2CollisionType::PolePlayer);
          earlierCollisionsTypes.push_back(co);
        }
      }
      else{
        R2CollisionTime co(kPolePlayers, R2CollisionType::PolePlayer);
        earlierCollisionsTypes.push_back(co);
      }
    }

    std::vector<R2BallPlayerCollision> newBallPlayerColls;
    if(!env.ballCatched){
      newBallPlayerColls=findFirstBallPlayersCollisions(partialT, ballPlayerBlacklist);
      bool collBall=(newBallPlayerColls.size()>0);
      double kBall=0.0;
      if(collBall){
        kBall=newBallPlayerColls[0].t;
      }
      collisions|=collBall;
      if(collBall){
        if(earlierCollisionsTypes.size()>0){
          double earlierT=earlierCollisionsTypes[0].t;
          if(kBall<earlierT){
            earlierCollisionsTypes.clear();
          }
          if(kBall<=earlierT){
            R2CollisionTime co(kBall, R2CollisionType::BallPlayer);
            earlierCollisionsTypes.push_back(co);
          }
        }
        else{
          R2CollisionTime co(kBall, R2CollisionType::BallPlayer);
          earlierCollisionsTypes.push_back(co);
        }
      }
    }

    std::vector<R2PlayerPlayerCollision> newPlayerPlayerColls=findFirstPlayerPlayersCollisions(partialT, playerPlayerCollisions);
    bool collPlayers=(newPlayerPlayerColls.size()>0);
    double kPlayers=0.0;
    if(collPlayers){
      kPlayers=newPlayerPlayerColls[0].t;
    }
    collisions|=collPlayers;

    if(collPlayers){
      if(earlierCollisionsTypes.size()>0){
        double earlierT=earlierCollisionsTypes[0].t;
        if(kPlayers<earlierT){
          earlierCollisionsTypes.clear();
        }
        if(kPlayers<=earlierT){
          R2CollisionTime co(kPlayers, R2CollisionType::PlayerPlayer);
          earlierCollisionsTypes.push_back(co);
        }
      }
      else{
        R2CollisionTime co(kPlayers, R2CollisionType::PlayerPlayer);
        earlierCollisionsTypes.push_back(co);
      }
    }

    if(collisions){
      double earlierT=earlierCollisionsTypes[0].t;
      addT=earlierT*(1.0 - partialT);
      updateMotion(addT);

      for(auto& co: earlierCollisionsTypes){
        if(co.type==R2CollisionType::BallPlayer){
          for(auto bpcoll:newBallPlayerColls){
            //avoiding continuous bouncing and ball entering the player
            ballPlayersColls[howManyBallPlayersColls]=bpcoll.p;
            ballPlayersCollsTeam[howManyBallPlayersColls]=bpcoll.team;
            howManyBallPlayersColls++;

            if(howManyBallPlayersColls>=4){
              if( (ballPlayersColls[howManyBallPlayersColls-1]==ballPlayersColls[howManyBallPlayersColls-3]) &&
                (ballPlayersColls[howManyBallPlayersColls-2]==ballPlayersColls[howManyBallPlayersColls-4]) &&
                (ballPlayersCollsTeam[howManyBallPlayersColls-1]==ballPlayersCollsTeam[howManyBallPlayersColls-3]) &&
                (ballPlayersCollsTeam[howManyBallPlayersColls-2]==ballPlayersCollsTeam[howManyBallPlayersColls-4])
                ){

                env.ball.velocity.zero();

                auto& p1= env.teams[int(ballPlayersCollsTeam[howManyBallPlayersColls-1])][ballPlayersColls[howManyBallPlayersColls-1]];
                p1.velocity.zero();
                auto& p2= env.teams[int(ballPlayersCollsTeam[howManyBallPlayersColls-2])][ballPlayersColls[howManyBallPlayersColls-2]];
                p2.velocity.zero();

                count++;
                continue;
              }
            }
            env.lastTouchedTeam2=bool(bpcoll.team);
            //let's change ball velocity
            auto& p= env.teams[bpcoll.team][bpcoll.p];
            Vec2 v=env.ball.velocity - p.velocity;
            double vel=v.len();
            Vec2 d=env.ball.pos - p. pos;
            if( ((v.x==0.0)&&(v.y==0.0)) || ((d.x==0.0)&&(d.y==0.0)) ){  // this happens if the ball is moving exactly at the same velocity as the player, they just intersected and floating numbers have some rounding errors
              // blacklist the player so it won't be checked continuously for the collision
              ballPlayerBlacklist[t1size*bpcoll.team+bpcoll.p]=true;
              count++;
              continue;
            }
            double impactAngle=atan2(d.y, d.x); // angle of the impact point on the player's circle wrt to player center
            double trajectoryAngle=atan2(v.y, v.x); // ball trajectory angle is the same as ball velocity direction, with inverted sign to have it on the same orientation of the d vector
            double reflectedAngle=impactAngle + remainder(impactAngle-trajectoryAngle, M_PI);
          
            double rX=cos(reflectedAngle);
            double rY=sin(reflectedAngle);
            
            if(vel > 0.0){
              if(! sett.simplified){
                if( fabs(remainder(p.direction-impactAngle, 2*M_PI)) < KickableAngle ){  // bounces on the back of a player, stopped in front of the player
                  env.ball.velocity.x=p.velocity.x*BallPlayerHitFactor +rX*vel*BallPlayerStopFactor;
                  env.ball.velocity.y=p.velocity.y*BallPlayerHitFactor +rY*vel*BallPlayerStopFactor;
                }
                else{
                  env.ball.velocity.x=p.velocity.x*BallPlayerHitFactor +rX*vel*BallPlayerBounceFactor;
                  env.ball.velocity.y=p.velocity.y*BallPlayerHitFactor +rY*vel*BallPlayerBounceFactor;
                }
              }
              else{ // simplified model
                if( fabs(remainder(p.direction-impactAngle, 2*M_PI)) < KickableAngle ){  // bounces on the back of a player, stopped in front of the player
                  env.ball.velocity.x=p.velocity.x*BallPlayerHitFactorSimplified;
                  env.ball.velocity.y=p.velocity.y*BallPlayerHitFactorSimplified;
                }
                else{
                  env.ball.velocity.x=p.velocity.x*BallPlayerHitFactorSimplified +rX*vel*BallPlayerBounceFactor;
                  env.ball.velocity.y=p.velocity.y*BallPlayerHitFactorSimplified +rY*vel*BallPlayerBounceFactor;
                }
              }
            }
          }
        }
        else if(co.type==R2CollisionType::PlayerPlayer){
          for(auto ppcoll:newPlayerPlayerColls){
            playerPlayerCollisions[ppcoll.team1*t1size+ppcoll.p1]=ppcoll.team2*t1size+ppcoll.p2;  // take note of collision
            playerPlayerCollisions[ppcoll.team2*t1size+ppcoll.p2]=ppcoll.team1*t1size+ppcoll.p1;

            //let's change players velocity
            auto& p1= env.teams[ppcoll.team1][ppcoll.p1];
            auto& p2= env.teams[ppcoll.team2][ppcoll.p2];
            // the mass is supposed equal for players.
            // modification to the first player
            Vec2 v1=p1.velocity - p2.velocity;  // velocity of p1 wrt p2 (just like p2 was still), that is total velocity of p1 impacting AGAINST p2
            Vec2 d1=p2.pos - p1.pos;
            double transmission=v1.cosBetween(d1) *0.5;  // the more the impact angle (depending on impact point, or the centers) coincides with the resulting relative velocity, the more the energy is transferred to the impact. It has to be divided by two (an half for each player)
            double momentum=v1.len()*transmission;  // the momentum depends on the resulting velocity magnitude, on the transmission
            // a part of inverted velocity goes to acceleration/deceleration depending on how much the direction coincides with the player direction
            Vec2 v2=v1*(-1);  // actual velocity vector of the impact AGAINST p1
            Vec2 dir1= Vec2(p1.direction) ;
            double accelPart1=v2.cosBetween(dir1);  // part of momentum to be used to accelerate/decelerate the player because aligned with its direction (the other part would displace a little the player)

            Vec2 dir2= Vec2(p2.direction) ;
            double accelPart2=v1.cosBetween(dir2);

            Vec2 accel1= dir1 * accelPart1 * momentum;
            Vec2 accel2= dir2 * accelPart2 * momentum;
            Vec2 displace1 = v2 * transmission -accel1 ; // ciò che rimane viene usato per l'accelerazione di tipo "displace" che è minore
            Vec2 displace2 = v1 * transmission -accel2 ;

            //Vec2 oldV1=p1.velocity;
            //Vec2 oldV2=p2.velocity;

            p1.velocity += accel1;
            p2.velocity += accel2;
            
            p1.velocity += displace1*CollisionPlayerDisplaceFactor;
            p2.velocity += displace2*CollisionPlayerDisplaceFactor;

            // let's stop completely the player if he crashed frontally
            /*
            // this was working decently
            if(accelPart1<0.0)
              p1.velocity.zero();
            if(accelPart2<0.0)
              p2.velocity.zero();
              */
            
            /*
            // this was working good enough
            if(d1.cosBetween(oldV1)<0.0)
              p1.velocity.zero();
            if(d1.cosBetween(oldV2)>0.0)
              p2.velocity.zero();
              */

            /*
            // this was working well and it's a little more principled and better working than the one above
            if(d1.cosBetween(p1.velocity)<0.0)
              p1.velocity.zero();
            if(d1.cosBetween(p2.velocity)>0.0)
              p2.velocity.zero();
              */

            // cancel only the frontal crash velocity component and not all the velocity: it results in a lesser blocking of players than the commented mechanisms above 
            d1.resize(1.0);
            Vec2 d2=d1*-1;
            double cosV1=d1.cosBetween(p1.velocity);
            if(cosV1>0.0){
              Vec2 toSub=d1*cosV1*p1.velocity.len();  // projection of p1 velocity onto the line connecting p1 and 2
              p1.velocity -= toSub;                   // subtract the crash direction component of the velocity
            }
            double cosV2=d2.cosBetween(p2.velocity);
            if(cosV2>0.0){
              Vec2 toSub=d2*cosV2*p2.velocity.len();  // projection of p2 velocity onto the line connecting p1 and 2
              p2.velocity -= toSub;                   // subtract the crash direction component of the velocity
            }
          }
        }
        else if(co.type==R2CollisionType::PoleBall){
          Vec2 pole=pitch.poles[newPoleBallColl.pole];

          double vel=env.ball.velocity.len();
          Vec2 d=env.ball.pos - pole;

          double impactAngle=atan2(d.y, d.x); // angle of the impact point on the pole circle wrt to pole center
          double trajectoryAngle=atan2(env.ball.velocity.y, env.ball.velocity.x); 
          double reflectedAngle=impactAngle + remainder(impactAngle-trajectoryAngle, M_PI);
          double rX=cos(reflectedAngle);
          double rY=sin(reflectedAngle);
          env.ball.velocity.x=rX*vel*BallPoleBounceFactor;
          env.ball.velocity.y=rY*vel*BallPoleBounceFactor;
          
        }
        else if(co.type==R2CollisionType::PolePlayer){
            for(auto ppcoll: newPolePlayersColls){
              auto& p1= env.teams[ppcoll.team][ppcoll.p];
              p1.velocity.zero();

              Vec2 pole=pitch.poles[ppcoll.pole];

              Vec2 d=(p1.pos-pole); // let's put just a little distance from the pole
              d.resize(sett.poleRadius+sett.playerRadius+R2Epsilon);
              p1.pos=pole+d;
            }

        }
      }

      partialT+=addT;
    }
    count ++;

    if(checkGoalOrBallOut()) {
      collisions=false;
    }
    else {
      oldEnv = env;
      manageBallInsidePlayers();
      if(checkGoalOrBallOut()) {
        collisions=false;
      }
      else{
        oldEnv = env;
      }
    }
  }

  if(env.ballCatched){
    setBallCatchedPosition();
  }

  //final series of checks
  if (env.state == R2State::Play) {
      updateMotion(1.0 - partialT); // the rest of the tick has to be completed
      addBallNoise();

      if (! checkGoalOrBallOut()) {
          oldEnv = env;
          manageBallInsidePlayers();
          if (! checkGoalOrBallOut()) {
            oldEnv = env;
          }
      } 
  }
}

void R2Simulator::manageBallInsidePlayers(){
  const double radius=sett.playerRadius+sett.ballRadius;
  if(env.ballCatched)
    return;
  bool collisions=true;

  auto checkCol=[&](int team){
    for(auto& p: env.teams[team])
    {
      Vec2 d=env.ball.pos - p.pos;

      double len=d.len();
      if((len+R2SmallEpsilon)< radius){
        collisions=true;

        d.resize(radius+R2SmallEpsilon);
        Vec2 oldPos=env.ball.pos;
        env.ball.pos=p.pos+d;
        Vec2 displace=env.ball.pos-oldPos;
        env.ball.velocity += displace*sett.ballInsidePlayerVelocityDisplace;

      }
    }
  };

  int count=0;
  while(collisions && (count <MaxCollisionLoop)){
    collisions=false;
    checkCol(0);
    checkCol(1);
    count ++;
  }
}

void R2Simulator::updateCollisionsAndMovements(){
  manageCollisions();
  manageStaticPlayersCollisions();
}

// here in case of collision the player is moved
void R2Simulator::manageStaticBallCollisions(){
  bool collisions=true;
  int count=0;
  while(collisions && (count <MaxCollisionLoop)){
    collisions=false;
    
    for(int team=0; team<=1; team++)
      for(auto& p: env.teams[team])
      {
        auto [dist, d]=p.dist(env.ball); 
        if(dist<R2Epsilon) {
          collisions=true;
          double angle=uniformDist(rng)*2*M_PI;
          double sx=cos(angle)*(sett.playerRadius+sett.ballRadius);
          double sy=sin(angle)*(sett.playerRadius+sett.ballRadius);
            p.pos.x-=sx;
            p.pos.y-=sy;
        }
        else {
          double ratio=(sett.playerRadius+sett.ballRadius)/dist;
          double diff=ratio-1.0;
          if(diff>0.0){
            collisions=true;
            p.pos.x+=d.x*diff;
            p.pos.y+=d.y*diff;
          }
        }
      }

    collisions|=manageStaticPoleBallCollisions();
    count ++;
  }
}

// player vs player collisions, when the game is inactive
void R2Simulator::manageStaticPlayersCollisions(){
  bool collisions=true;

  auto checkCol=[&](int t1, int t2){
    
    int c1=0;
    for(auto& p1: env.teams[t1]){
      c1++;
      int c2=0;
      for(auto& p2: env.teams[t2]){
        c2++;
        if(&p1 == &p2)
          continue;

        auto [dist, d]=p2.dist(p1); 

        if(dist<R2Epsilon) {
          collisions=true;
          double angle=uniformDist(rng)*2*M_PI;
          double sx=cos(angle)*sett.playerRadius;
          double sy=sin(angle)*sett.playerRadius;
            p1.pos.x+=sx;
            p1.pos.y+=sy;
            p2.pos.x-=sx;
            p2.pos.y-=sy;
        }
        else if(dist<sett.playerRadius*2){
            double ratio=sett.playerRadius*2/dist;
            collisions=true;
            double toAdd= ratio-1.0;
            
            p1.pos.x+=d.x*toAdd*0.5;
            p1.pos.y+=d.y*toAdd*0.5;
            p2.pos.x-=d.x*toAdd*0.5;
            p2.pos.y-=d.y*toAdd*0.5;

        }
        
      }
    }
  };

  int count=0;
  while(collisions && (count <MaxCollisionLoop)){
    collisions=false;
    checkCol(0, 1);
    checkCol(0, 0);
    checkCol(1, 1);
    collisions|=manageStaticPolePlayersCollisions();
    count ++;
  }
}

bool R2Simulator::manageStaticPoleBallCollisions(){
  const double radius=sett.poleRadius+sett.ballRadius;

  for(auto& pole: pitch.poles)
  {
    Vec2 d=env.ball.pos-pole; 
    double dist=d.len();
    if(dist<R2Epsilon) {
      double angle=uniformDist(rng)*2*M_PI;
      double sx=cos(angle)*radius;
      double sy=sin(angle)*radius;
        env.ball.pos.x-=sx;
        env.ball.pos.y-=sy;
        return true;
    }
    else {
      double ratio=radius/dist;
      double diff=ratio-1.0;
      if(diff>0.0){
        env.ball.pos.x+=d.x*diff;
        env.ball.pos.y+=d.y*diff;
        return true;
      }
    }
  }
  return false;
}

bool R2Simulator::manageStaticPolePlayersCollisions(){
  const double radius=sett.poleRadius+sett.playerRadius;
  bool collisions=true;

  int count=0;
  while(collisions && (count <MaxCollisionLoop)){
    collisions=false;
    
    for(int team=0; team<=1; team++)
      for(auto& p: env.teams[team])
      {
        for(auto& pole: pitch.poles)
        {
          Vec2 d=p.pos-pole; 
          double dist=d.len();
          if(dist<R2Epsilon) {
            collisions=true;
            double angle=uniformDist(rng)*2*M_PI;
            double sx=cos(angle)*radius;
            double sy=sin(angle)*radius;
              p.pos.x-=sx;
              p.pos.y-=sy;
          }
          else {
            double ratio=radius/dist;
            double diff=ratio-1.0;
            if(diff>0.0){
              collisions=true;
              p.pos.x+=d.x*diff;
              p.pos.y+=d.y*diff;
            }
          }
        }
      }

    count ++;
  }
  return collisions;
}

void R2Simulator::preState(){
  switch(env.state)
  {
    case R2State::Inactive: 
      break;
    case R2State::Ready:  // currently unused
      break;
    case R2State::Kickoff1:
      env.ballCatched=0;
      env.ball.pos.zero();
      env.ball.velocity.zero();
      break;
    case R2State::Kickoff2:
      env.ballCatched=0;
      env.ball.pos.zero();
      env.ball.velocity.zero();
      break;
    case R2State::Play:
      if(env.ballCatched){
        if(!isPlayerInsideHisArea(0, int(env.ballCatchedTeam2))){
          env.ballCatched=0;  //goalkeeper exited his area, balls drop
        }
        else{
          env.ballCatched--;
        }
        
        if(env.ballCatched==0){
          setBallReleasedPosition();  // position ball in front of goalkeeper
        }
      }
      ballAlreadyKicked=false;
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Stopped:
      env.ballCatched=0;
      setBallReleasedPosition();
      env.ball.velocity.zero();
      break;
    case R2State::Goalkick1up:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.goalKickLx;
      else
        env.ball.pos.x=pitch.goalKickRx;
      env.ball.pos.y=pitch.goalKickUy;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Goalkick1down:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.goalKickLx;
      else
        env.ball.pos.x=pitch.goalKickRx; 
      env.ball.pos.y=pitch.goalKickDy;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Goalkick2up:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.goalKickRx;
      else
        env.ball.pos.x=pitch.goalKickLx;
      env.ball.pos.y=pitch.goalKickUy;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Goalkick2down:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.goalKickRx;
      else
        env.ball.pos.x=pitch.goalKickLx;
      env.ball.pos.y=pitch.goalKickDy;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Corner1up:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.x1;
      else
        env.ball.pos.x=pitch.x2;
      env.ball.pos.y=pitch.y1;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Corner1down:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.x1;
      else
        env.ball.pos.x=pitch.x2;
      env.ball.pos.y=pitch.y2;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Corner2up:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.x2;
      else
        env.ball.pos.x=pitch.x1;
      env.ball.pos.y=pitch.y1;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Corner2down:
      env.ballCatched=0;
      if(!env.halftimePassed) 
        env.ball.pos.x=pitch.x2;
      else
        env.ball.pos.x=pitch.x1;
      env.ball.pos.y=pitch.y2;
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Throwin1:
      env.ballCatched=0;
      setBallThrowInPosition();
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Throwin2:
      env.ballCatched=0;
      setBallThrowInPosition();
      env.ball.velocity.zero();
      env.startingTeamMaxRange=0.0;
      break;
    case R2State::Paused:
      break;
    case R2State::Halftime:
      env.ballCatched=0;
      break;
    case R2State::Goal1:
      break;
    case R2State::Goal2:
      break;
    case R2State::Ended:
      break;
    default:
      break;
  } 
}

void R2Simulator::checkState(){
  switch(env.state)
  {
    case R2State::Inactive: 
      break;
    case R2State::Ready: 
      break;
    case R2State::Kickoff1:
      limitPlayersToHalfPitchFullBody(0);
      manageStaticBallCollisions();
      manageStaticPlayersCollisions();
      manageStaticBallCollisions();
      limitPlayersToHalfPitchFullBody(0);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Kickoff2:
      limitPlayersToHalfPitchFullBody(1);
      manageStaticBallCollisions();
      manageStaticPlayersCollisions();
      manageStaticBallCollisions();
      limitPlayersToHalfPitchFullBody(1);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Play:
      updateCollisionsAndMovements();
      break;
    case R2State::Stopped:
      break;
    case R2State::Goalkick1up:
    case R2State::Goalkick1down:
      limitPlayersOutsideAreaFullBody(false);
      manageStaticBallCollisions();
      manageStaticPlayersCollisions();
      limitPlayersOutsideAreaFullBody(false);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Goalkick2up:
    case R2State::Goalkick2down:
      limitPlayersOutsideAreaFullBody(true);
      manageStaticBallCollisions();
      manageStaticPlayersCollisions();
      limitPlayersOutsideAreaFullBody(true);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Corner1up:
    case R2State::Corner1down:
      putPlayersFarFromBall(1, sett.cornerMinDistance);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Corner2up:
    case R2State::Corner2down:
      putPlayersFarFromBall(0, sett.cornerMinDistance);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Throwin1:
      putPlayersFarFromBall(1, sett.throwinMinDistance);
      oldEnv = env;
      env.state=R2State::Play;
      break;
    case R2State::Throwin2:
      putPlayersFarFromBall(0, sett.throwinMinDistance);
      oldEnv = env;
      env.state=R2State::Play;
      break;

    case R2State::Paused:
      break;
    case R2State::Halftime:
      break;
    case R2State::Goal1:
      oldEnv = env;
      env.state=R2State::Kickoff2;
      break;
    case R2State::Goal2:
      oldEnv = env;
      env.state=R2State::Kickoff1;
      break;
    case R2State::Ended:
      break;
    default:
      break;
  } 
}

void R2Simulator::setStartMatch() {
  startedTeam2=false;
  if(uniformDist(rng) >= 0.5)
    startedTeam2=true;
  env.state= startedTeam2 ? R2State::Kickoff2 : R2State::Kickoff1;
}

void R2Simulator::setHalfTime() {
    env.state= (!startedTeam2) ? R2State::Kickoff2 : R2State::Kickoff1;
    env.halftimePassed= true;
}

/*
// this was working well too.
void R2Simulator::playMatch() {
  setStartMatch();
  for(int i=0; i<2; i++)
  {
    for(int t=0; t<sett.ticksPerTime; t++)
    {
        step();
    }
    setHalfTime();
  }
  history.envs.back()=env;  // log also final environment
}
*/

void R2Simulator::playMatch() {
  while(stepIfPlaying());
}

bool R2Simulator::stepIfPlaying(){
  if(env.tick==0)
	  setStartMatch();
  else if(env.tick==sett.ticksPerTime)
	  setHalfTime();

  if(env.tick<(sett.ticksPerTime*2)){
    step();
    return true;
  }
  else if(env.tick==(sett.ticksPerTime*2)){
      history.envs.back()=env;  // log also final environment
      env.tick++;
      env.state=R2State::Ended;
	}
  else if(env.tick>(sett.ticksPerTime*2)){  //just in case is reached by setEnvironment()
      env.state=R2State::Ended;
	}
  return false;
}

void R2Simulator::processStep(const R2Action& action, int team, int player){
  history.actions[env.tick][processedActions].team=team;
  history.actions[env.tick][processedActions].action=action;
  history.actions[env.tick][processedActions].player=player;
  processedActions++;
 
  switch(env.state)
  {
    case R2State::Inactive: 
      break;
    case R2State::Ready:  // currently, an unnecessary state
      procReady(action, team, player); 
      break;
    case R2State::Kickoff1:
    case R2State::Kickoff2:
      procKickoff(action, team, player);
      break;
    case R2State::Play:
      procPlay(action, team, player);
      break;
    case R2State::Stopped:
      break;
    case R2State::Goalkick1up:
    case R2State::Goalkick1down:
    case R2State::Goalkick2up:
    case R2State::Goalkick2down:
      procGoalkick(action, team, player);
      break;
    case R2State::Corner1up:
    case R2State::Corner1down:
    case R2State::Corner2up:
    case R2State::Corner2down:
      procCorner(action, team, player);
      break;
    case R2State::Throwin1:
    case R2State::Throwin2:
      procThrowin(action, team, player);
      break;
    case R2State::Paused:
      break;
    case R2State::Halftime:
      break;
    case R2State::Goal1:
    case R2State::Goal2:
      procGoal(action, team, player);
      break;
    case R2State::Ended:
      procEnded(action, team, player);
      break;
    default:
      break;
  } 
}

// we check the center of the body of the player
void R2Simulator::limitPlayersOutsideArea(int kickTeam){
  int i=0;
  int team=1-kickTeam;
  double defaultX= ((kickTeam && (!env.halftimePassed) ) || ( (!kickTeam) && env.halftimePassed) ) ? pitch.areaRx : pitch.areaLx;
  for(auto& p: env.teams[team]){
    if(isPlayerInsideOpponentArea(i, team)){
      p.pos.x=defaultX;
    }
    i++;
  }
}

void R2Simulator::limitPlayersOutsideAreaFullBody(int kickTeam){
  int i=0;
  int team=1-kickTeam;
  double defaultX= ((kickTeam && (!env.halftimePassed) ) || ( (!kickTeam) && env.halftimePassed) )? (pitch.areaRx-sett.playerRadius) : (pitch.areaLx+sett.playerRadius);
  for(auto& p: env.teams[team]){
    if(isPlayerInsideOpponentAreaFullBody(i, team)){
      p.pos.x=defaultX;
    }
    i++;
  }
}

// we check the complete diameter of the body of the player
void R2Simulator::limitPlayersToHalfPitch(int kickTeam, double dueDistance){
  for(auto& p: env.teams[0]){
    if (!env.halftimePassed){
      if (p.pos.x >0.0)
        p.pos.x=0.0;
    }
    else{
      if(p.pos.x <0.0)
        p.pos.x=0.0;
    }
  }

  for(auto&p: env.teams[1]){
    if (!env.halftimePassed){
      if(p.pos.x <0.0)
        p.pos.x=0.0;
    }
    else{
      if (p.pos.x >0.0)
        p.pos.x=0.0;
    }
  }
  
  for(auto& p: env.teams[1-kickTeam]){
    double dist=p.absDistanceFromCenter();
    if(dist<=R2Epsilon){
      p.pos.x=(-2*kickTeam+1)*dueDistance;  //to have sign - only if kickteam is 1
      p.pos.y=(uniformDist(rng)-0.5)*dueDistance;
    }
    else if (dist < dueDistance)
    {
      double ratio=dueDistance/dist;
      p.pos.x*=ratio;
      p.pos.y*=ratio;
    }
  }
  
}

void R2Simulator::procReady(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      actionMove(action, team, player);
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procKickoff(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      actionMoveKickoff(action, team, player);
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procPlay(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      break;
    case  R2ActionType::Dash:
      actionDash(action, team, player);
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      actionKick(action, team, player);
      break;
    case  R2ActionType::Catch:
      actionCatch(action, team, player);
      break;
    default:
      break;
  }
}

void R2Simulator::procGoalkick(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      actionMoveGoalkick(action, team, player);
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procCorner(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      actionMoveCorner(action, team, player);
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procThrowin(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      actionMoveThrowin(action, team, player);
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procGoal(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

void R2Simulator::procEnded(const R2Action& action, int team, int player){
  switch(action.action){
    case  R2ActionType::NoOp:
      break;
    case  R2ActionType::Move:
      break;
    case  R2ActionType::Dash:
      break;
    case  R2ActionType::Turn:
      break;
    case  R2ActionType::Kick:
      break;
    case  R2ActionType::Catch:
      break;
    default:
      break;
  }
}

std::vector<std::string> R2Simulator::getTeamNames(){
  return std::vector<std::string>{ teamNames[0], teamNames[1]};
}

std::string R2Simulator::getStateString(){
  std::string s= teamNames[0]+(env.halftimePassed ? " (right) " : " (left) ")+ "vs " + teamNames[1]+(env.halftimePassed ? " (left) " : " (right) ")+ to_string(env.score1)+"-"+to_string(env.score2)+" "+"tick:"+to_string(env.tick)+" ";
  switch(env.state)  {
    case R2State::Inactive: 
      return s+string("Inactive");
    case R2State::Ready:  // currently, an unnecessary state
      return s+string("Ready");
    case R2State::Kickoff1:
      return s+string("Kickoff1");
    case R2State::Kickoff2:
      return s+string("Kickoff2");
    case R2State::Play:
      return s+string("Play");
    case R2State::Stopped:
      return s+string("Stopped");
    case R2State::Goalkick1up:
      return s+string("Goalkick1up");
    case R2State::Goalkick1down:
      return s+string("Goalkick1down");
    case R2State::Goalkick2up:
      return s+string("Goalkick2up");
    case R2State::Goalkick2down:
      return s+string("Goalkick2down");
    case R2State::Corner1up:
      return s+string("Corner1up");
    case R2State::Corner1down:
      return s+string("Corner1down");
    case R2State::Corner2up:
      return s+string("Corner2up");
    case R2State::Corner2down:
      return s+string("Corner2down");
    case R2State::Throwin1:
      return s+string("Throwin1");
    case R2State::Throwin2:
      return s+string("Throwin2");
    case R2State::Paused:
      return s+string("Paused");
    case R2State::Halftime:
      return s+string("Halftime");
    case R2State::Goal1:
      return s+string("Goal1");
    case R2State::Goal2:
      return s+string("Goal2");
    case R2State::Ended:
      return s+string("Ended");
    default:
      return s+string("Unknown");
  }
}
 
std::string R2Simulator::createDateFilename(){
    char string_buf[90];
    time_t simpletime;
    struct tm * timeloc;

    time (&simpletime);
    timeloc = localtime(&simpletime);

    strftime(string_buf, sizeof(string_buf),"%Y-%m-%d %H-%M-%S",timeloc);
    std::string filename(string_buf);
    sprintf(string_buf,"_%u", random_seed);
    filename.append(string_buf);
    return filename;
} 

bool R2Simulator::saveStatesHistory(std::string filename){
  ofstream myfile;
  myfile.open (filename);
  if (!myfile.is_open())
    return false;

  myfile << R2SVersion << std::endl;

  myfile << teamNames[0] << std::endl;
  myfile << teamNames[1] << std::endl;

  myfile << env.teams[0].size() << "," << env.teams[1].size() << std::endl;

  myfile << sett.ticksPerTime;
  myfile << "," << sett.pitchLength;
  myfile << "," << sett.pitchWidth;
  myfile << "," << sett.goalWidth;
  myfile << "," << sett.centerRadius;
  myfile << "," << sett.poleRadius;   
  myfile << "," << sett.ballRadius;
  myfile << "," << sett.playerRadius;
  myfile << "," << sett.catchRadius;
  myfile << "," << sett.catchHoldingTicks;
  myfile << "," << sett.kickRadius;
  myfile << "," << sett.kickableDistance;
  myfile << "," << sett.catchableDistance;
  myfile << "," << sett.kickableAngle;
  myfile << "," << sett.kickableDirectionAngle;
  myfile << "," << sett.catchableAngle;
  myfile << "," << sett.netLength;
  myfile << "," << sett.catchableAreaLength;
  myfile << "," << sett.catchableAreaWidth;
  myfile << "," << sett.cornerMinDistance;
  myfile << "," << sett.throwinMinDistance;
  myfile << "," << sett.outPitchLimit;
  myfile << "," << sett.maxDashPower;
  myfile << "," << sett.maxKickPower;
  myfile << "," << sett.playerVelocityDecay;
  myfile << "," << sett.ballVelocityDecay;
  myfile << "," << sett.maxPlayerSpeed;
  myfile << "," << sett.maxBallSpeed;
  myfile << "," << sett.catchProbability;
  myfile << "," << sett.playerRandomNoise;
  myfile << "," << sett.playerDirectionNoise;
  myfile << "," << sett.playerVelocityDirectionMix;
  myfile << "," << sett.ballInsidePlayerVelocityDisplace;
  myfile << "," << sett.afterCatchDistance;
  myfile << std::endl;

  for (int tick=0; tick<history.envs.size(); tick++){
    auto& env=history.envs[tick];

    myfile << tick << ",";
    myfile << env.score1 << ",";
    myfile << env.score2 << ",";
    myfile << int(env.state) << ",";

    myfile << env.ball.pos.x << "," << env.ball.pos.y << "," << env.ball.velocity.x << "," << env.ball.velocity.y << "," ;

    for(auto& p1: env.teams[0])
      myfile << p1.pos.x << "," << p1.pos.y << "," << p1.velocity.x << "," << p1.velocity.y << "," << p1.direction << "," ;
    
    for(auto& p2: env.teams[1])
      myfile << p2.pos.x << "," << p2.pos.y << "," << p2.velocity.x << "," << p2.velocity.y << "," << p2.direction << "," ;

    myfile << env.lastTouchedTeam2 << ",";
    myfile << env.startingTeamMaxRange << ",";
    myfile << env.ballCatched << ",";
    myfile << env.ballCatchedTeam2;

    myfile << std::endl;
  }

  myfile.close();
  return true;
}

bool R2Simulator::saveActionsHistory(std::string filename){
  ofstream myfile;
  myfile.open (filename);
  if (!myfile.is_open())
    return false;

  for (int tick=0; tick<history.actions.size(); tick++)
    for(auto& actionPack : history.actions[tick]){
      myfile << tick << "," << actionPack.team << "," << actionPack.player << "," << int(actionPack.action.action) << "," 
        << actionPack.action.data[0] << "," << actionPack.action.data[1] << "," << actionPack.action.data[2] << std::endl;
    }

  myfile.close();
  return true;
}


void R2Simulator::setEnvironment(int _tick, int _score1, int _score2, R2State _state, R2ObjectInfo _ball, 
    std::vector<R2PlayerInfo> _team1, std::vector<R2PlayerInfo> _team2,
    bool _lastTouchedTeam2, int _ballCatched, bool _ballCatchedTeam2) {
        if (_tick >= (2*sett.ticksPerTime))
            _tick=2*sett.ticksPerTime;

        env.tick=_tick;
        env.score1=_score1;
        env.score2=_score2;
        env.state=_state;
        env.ball=_ball;
        env.teams[0]=_team1;
        env.teams[1]=_team2;
        env.lastTouchedTeam2=_lastTouchedTeam2;
        env.ballCatched=_ballCatched;
        env.ballCatchedTeam2=_ballCatchedTeam2;

        if(env.tick>=sett.ticksPerTime)
            env.halftimePassed=true;

        oldEnv = env;
        oldEnv.state= R2State::Inactive;
  }

} //end namespace