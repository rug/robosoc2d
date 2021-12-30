// (c) 2021 Ruggero Rossi
// a simple, reactive, robosoc2d player agent (with no planning)
#include <iostream>
#include "simple_player.h"

using namespace std;
using namespace r2s;

// explanation: http://paulbourke.net/geometry/pointlineplane/
// p is the point
// s1 and s2 are the segment's ends
double distancePointSegment(Vec2 p, Vec2 s1, Vec2 s2)
{
    Vec2 d=s2-s1;
    Vec2 dp1=p-s1; 

    //if the segment is collapsed in one point we can't use the formula but we can still calculate point to point distance
    if ( (d.x >= -R2SmallEpsilon) && (d.x <= R2SmallEpsilon) &&
        (d.y >= -R2SmallEpsilon) && (d.y <= R2SmallEpsilon) )
        return dp1.len();
    
    double l=d.len();
    double u= (dp1.x*d.x + dp1.y*d.y)/(l*l);

    if( (u<0.0) || (u>1.0) ){   // perpendicular intersection would be outside segment. Take instead the shorter value among the two distances between the point and the two segment's ends 
        Vec2 dp2=p-s2;
        return min(dp1.len(), dp2.len());
    }

    Vec2 intersection(s1.x+u*d.x , s1.y+u*d.y);
    return (p-intersection).len();
}

namespace r2s {

SimplePlayer::SimplePlayer(int index, int _whichTeam) : shirtNumber(index), team(_whichTeam),
 normalEnv(), env(), prevAction(), prevState(R2State::Inactive),
 uniformDist(0.0, 1.0), rng (42), cosKickableAngle(0.0), cosCatchableAngle(0.0) {
};

R2Action  SimplePlayer::transformActionIfNecessary(R2Action action){
    if((team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed)){
        switch(action.action){
            case R2ActionType::Move :
                action.data[0]=-action.data[0];
                action.data[1]=-action.data[1];
                action.data[2]=fixAnglePositive(action.data[2] + M_PI);
            break;            
            case R2ActionType::Dash :
            case R2ActionType::Turn :
            case R2ActionType::Kick :
            case R2ActionType::Catch :
                action.data[0]=fixAnglePositive(action.data[0] + M_PI);
            break;
            default:
            break;
        }
    }
    return action;
}

void  SimplePlayer::transformEnvIfNecessary(){
    env=normalEnv;
    if(team) {
        env.teams[0]=normalEnv.teams[1];  // just in case they have a different number of players in the team
        env.teams[1]=normalEnv.teams[0];
        
        env.score1=normalEnv.score2;
        env.score2=normalEnv.score1;
        env.lastTouchedTeam2=!env.lastTouchedTeam2; // teams[1] thinks to be teams[0]
        env.ballCatchedTeam2=!env.ballCatchedTeam2;
        // now invert the state
        R2State state=env.state;
        switch(state){
            case R2State::Kickoff1 :
                env.state=R2State::Kickoff2 ;
            break;
            case R2State::Kickoff2 :
                env.state=R2State::Kickoff1 ;
            break;
            case R2State::Goalkick1up :
                env.state=R2State::Goalkick2down ;
            break;
            case R2State::Goalkick1down :
                env.state=R2State::Goalkick2up ;
            break;
            case R2State::Goalkick2up :
                env.state=R2State::Goalkick1down ;
            break;
            case R2State::Goalkick2down :
                env.state=R2State::Goalkick1up ;
            break;
            case R2State::Corner1up :
                env.state=R2State::Corner2down ;
            break;
            case R2State::Corner1down :
                env.state=R2State::Corner2up ;
            break;
            case R2State::Corner2up :
                env.state=R2State::Corner1down ;
            break;
            case R2State::Corner2down :
                env.state=R2State::Corner1up ;
            break;
            case R2State::Throwin1 :
                env.state=R2State::Throwin2 ;
            break;
            case R2State::Throwin2 :
                env.state=R2State::Throwin1 ;
            break;
            case R2State::Goal1:
                env.state=R2State::Goal2 ;
            break;
            case R2State::Goal2 :
                env.state=R2State::Goal1 ;
            break;
            default:
            break;
        }
    }

    if(env.halftimePassed) {
        R2State state=env.state;
        switch(state){
            case R2State::Goalkick1up :
                env.state=R2State::Goalkick1down ;
            break;
            case R2State::Goalkick1down :
                env.state=R2State::Goalkick1up ;
            break;
            case R2State::Goalkick2up :
                env.state=R2State::Goalkick2down ;
            break;
            case R2State::Goalkick2down :
                env.state=R2State::Goalkick2up ;
            break;
            case R2State::Corner1up :
                env.state=R2State::Corner1down ;
            break;
            case R2State::Corner1down :
                env.state=R2State::Corner1up ;
            break;
            case R2State::Corner2up :
                env.state=R2State::Corner2down ;
            break;
            case R2State::Corner2down :
                env.state=R2State::Corner2up ;
            break;
            default:
            break;
        }

    }

    if((team && (!env.halftimePassed) ) || ( (!team) && env.halftimePassed)){
        env.ball.pos.invert();
        env.ball.velocity.invert();
        for(int i=0; i<env.teams[0].size(); i++){
            env.teams[0][i].pos.invert();
            env.teams[0][i].velocity.invert();
            env.teams[0][i].direction=fixAnglePositive(env.teams[0][i].direction + M_PI);
        }
        for(int i=0; i<env.teams[1].size(); i++){
            env.teams[1][i].pos.invert();
            env.teams[1][i].velocity.invert();
            env.teams[1][i].direction=fixAnglePositive(env.teams[1][i].direction + M_PI);
        }
    }

}

double SimplePlayer::bestKickAnglePossible(double angle){
    double bestAngle=angle;
    if( (!isTeamResumeKicking()) && (!sett.simplified) ) {
        auto& p=env.teams[0][shirtNumber];
        double a=fixAnglePositive(angle);
        double d=fixAngleTwoSides(a-p.direction);
        if(d > sett.kickableDirectionAngle){
            bestAngle=p.direction+sett.kickableDirectionAngle;   
        }
        else if(d < -sett.kickableDirectionAngle){
            bestAngle=p.direction-sett.kickableDirectionAngle;
        }
    } 
    return bestAngle;
}

std::tuple<bool, double> SimplePlayer::isKickOrientedTowardOwnGoal(double angle){
    double a=fixAnglePositive(angle);
    if( (a<M_PI_2) || (a> M_PI*1.5))
        return std::tuple<bool, double>{false, 0.0};

    Vec2 dir=Vec2(a);
    if(dir.x==0.0)
        return std::tuple<bool, double>{false, 0.0};
    
    double y=env.ball.pos.y+ (pitch.x2- env.ball.pos.x)*dir.y/dir.x;
    if( (y<pitch.yGoal1) && (y>pitch.yGoal2) )
        return std::tuple<bool, double>{true, y};

    return std::tuple<bool, double>{false, 0.0};
}

double SimplePlayer::angleTowardPoint(double x, double y){
    auto& p= env.teams[0][shirtNumber];
    double dx=x - p.pos.x;
    double dy=y - p.pos.y;
    if((dx==0.0)&&(dy==0.0))
        return 0.0;
    return atan2(dy,dx);
}

double SimplePlayer::angleTowardBall(){
    return angleTowardPoint(env.ball.pos.x, env.ball.pos.y);
}

double SimplePlayer::angleTowardCenterGoal(){
    return angleTowardPoint(pitch.x1, 0.0);
}

double SimplePlayer::angleTowardUpperPole(){
    return angleTowardPoint(pitch.x1, pitch.yGoal1);
}

double SimplePlayer::angleTowardLowerPole(){
    return angleTowardPoint(pitch.x1, pitch.yGoal2);
}

double SimplePlayer::angleTowardOwnUpperPole(){
    return angleTowardPoint(pitch.x2, pitch.yGoal1);
}

double SimplePlayer::angleTowardOwnLowerPole(){
    return angleTowardPoint(pitch.x2, pitch.yGoal2);
}

double SimplePlayer::ballAngleTowardPoint(double x, double y){
    double dx=x - env.ball.pos.x;
    double dy=y - env.ball.pos.y;
    if((dx==0.0)&&(dy==0.0))
        return 0.0;
    return atan2(dy,dx);
}

double SimplePlayer::ballAngleTowardCompanion(int companion){
    return ballAngleTowardPoint(env.teams[0][companion].pos.x, env.teams[0][companion].pos.y);
}

double SimplePlayer::ballAngleTowardCenterGoal(){
    return ballAngleTowardPoint(pitch.x1, 0.0);
}

double SimplePlayer::ballAngleTowardUpperPole(){
    return ballAngleTowardPoint(pitch.x1, pitch.yGoal1);
}

double SimplePlayer::ballAngleTowardLowerPole(){
    return ballAngleTowardPoint(pitch.x1, pitch.yGoal2);
}

bool SimplePlayer::isTeamResumeKicking(){
    return ((prevState==R2State::Kickoff1) || (prevState==R2State::Goalkick1up) ||
     (prevState==R2State::Goalkick1down) || (prevState==R2State::Throwin1) ||
     (prevState==R2State::Corner1up) || (prevState==R2State::Corner1down) );
}

bool SimplePlayer::isBallKickable(){
    auto p= env.teams[0][shirtNumber]; 
    
    auto [dist, d]=p.dist(env.ball); 
    // is ball reachable
    if(dist>sett.kickableDistance)
        return false;

    // is ball in front of player? only when not in kicking from restart game.
    if( (!sett.simplified) && (!isTeamResumeKicking()) && (fabs(dist)>R2SmallEpsilon) ){
        double cosinusPlayerBall= (d.x*cos(p.direction) + d.y*sin(p.direction))/dist;
        if(cosinusPlayerBall <= cosKickableAngle)  
            return false;
    }

    return true;
}

// not considering the goalkeeper
Vec2 SimplePlayer::opponentBaricenter(){
    Vec2 sum;
    int i=1;
    for (; i<env.teams[1].size(); i++){
        sum+=env.teams[1][i].pos;
    }
    return sum*(1/i);
}

// except goalkeeper
bool SimplePlayer::isOpponentCloserToBall(){

    int closerTeam=0;
    double minDist=1000000.0;
    for(int team=0; team<=1; team++)
        for (int i=1; i<env.teams[team].size(); i++){
            auto diff=env.teams[team][i].pos - env.ball.pos;
            double dist=diff.x*diff.x+diff.y*diff.y;
            if(dist<minDist){
                closerTeam=team;
                minDist=dist;
            }
        }
    return (closerTeam==1);
}

// except goalkeeper
int SimplePlayer::playerClosestToBall(std::vector<R2PlayerInfo>& team){
    int closest =1;
    if (team.size() < 2)
        return 0;
    auto diff1=team[1].pos - env.ball.pos;
    double minDist= diff1.x*diff1.x+diff1.y*diff1.y;
    for (int i=2; i<team.size(); i++){
        auto diff=team[i].pos - env.ball.pos;
        double dist=diff.x*diff.x+diff.y*diff.y;
        if(dist<minDist){
            closest=i;
            minDist=dist;
        }
    }
    return closest;
}

// except goalkeeper
int SimplePlayer::myPlayerClosestToBall(){
    return playerClosestToBall(env.teams[0]);
}

// except goalkeeper
int SimplePlayer::opponentPlayerClosestToBall(){
    return playerClosestToBall(env.teams[1]);
}

//be careful: returns -1 if opposing team is empty
//currently unused method
int SimplePlayer::myClosestOpponent(){
    if(env.teams[1].size()<1)
        return -1;
    auto& myself=env.teams[0][shirtNumber];
    int closest =0;
    auto diff1=env.teams[1][0].pos - myself.pos;
    double minDist= diff1.x*diff1.x+diff1.y*diff1.y;
    for (int i=1; i<env.teams[1].size(); i++){
        auto diff=env.teams[1][i].pos - myself.pos;
        double dist=diff.x*diff.x+diff.y*diff.y;
        if(dist<minDist){
            closest=i;
            minDist=dist;
        }
    }
    return closest;
}

//be careful: returns -1 if its the only player in the team
//currently unused method
int SimplePlayer::myClosestCompanion(){
    auto& myself=env.teams[0][shirtNumber];
    int closest = -1;
    double minDist=1000000.0;
    for (int i=0; i<env.teams[0].size(); i++){
        if(i!=shirtNumber){
            auto diff=env.teams[0][i].pos - myself.pos;
            double dist=diff.x*diff.x+diff.y*diff.y;
            if(dist<minDist){
                closest=i;
                minDist=dist;
            }
        }
    }
    return closest;
}

int SimplePlayer::myPlayerClosestToOpponent(int opponent, std::vector<int> assigned){
    Vec2 pos=env.teams[1][opponent].pos;
    int closest =-1;
    double minDist=1000000.0;
    for (int i=0; i<assigned.size(); i++){
        if(assigned[i]==0){
            auto diff=pos - env.teams[0][i].pos;
            double dist=diff.x*diff.x+diff.y*diff.y;
            if(dist<minDist){
                closest=i;
                minDist=dist;
            }
        }
    }
    return closest;
}

double SimplePlayer::minOpponentSegmentDistance(Vec2 s1, Vec2 s2){
    double minSegmentDistance=1000.0;
    for(int i=0; i<env.teams[1].size(); i++){
        auto& p2=env.teams[1][i];
        double segmentDistance=distancePointSegment(p2.pos, s1, s2);
        minSegmentDistance=min(minSegmentDistance,segmentDistance);
    }
    return minSegmentDistance;
}

constexpr double passGoalkeeperDisadvantage=-10;
constexpr double passBackwardDisadvantage=-5;
constexpr double passAdvancedAdvantage=5;
constexpr double passTrajectoryOccludedDisadvantage=-20;
constexpr double passFavouriteDistance=10;
constexpr double passDistanceDisadvantage=-0.1;   
constexpr double advancedPassLength=5.0;
constexpr double outOfAngleDisadvantage=-25;
constexpr double ownGoalDistanceThreshold=6;
constexpr double ownGoalDistanceDisadvantage=-15;

Vec2 SimplePlayer::calcAdvancedPosition(int player){
    auto pos=env.teams[0][player].pos;
    Vec2 d=Vec2(pitch.x1, 0.0) - pos;
    d.resize(advancedPassLength); //segment from player to center of opposite goal
    pos+=d;
    return pos;
}

double SimplePlayer::companionPassValue(int player, bool advanced){
    auto pos= advanced ? calcAdvancedPosition(player) : env.teams[0][player].pos;
    double value=0.0;

    if(advanced){
        value+=passAdvancedAdvantage;
    }

    double minSegmentDistance=minOpponentSegmentDistance(pos, env.ball.pos);
    
    if(minSegmentDistance<= (sett.ballRadius+sett.playerRadius))
        value+=passTrajectoryOccludedDisadvantage;

    if(player==0)
        value+=passGoalkeeperDisadvantage;

    if(pos.x < env.ball.pos.x)
        value+=passBackwardDisadvantage;

    double dist= (env.ball.pos - pos).len();
    value+=fabs(passFavouriteDistance-dist)*passDistanceDisadvantage;

    // calc if kicking angle is possible
    bool outOfAngle=false;
    Vec2 d=pos -env.ball.pos;
    double angle=0.0;
    if( (!sett.simplified) && (d.x!=0.0)&&(d.y!=0.0)){
        angle=atan2(d.y,d.x);
        if(!isTeamResumeKicking()){
            auto& p=env.teams[0][shirtNumber];
            double a=fixAnglePositive(angle);
            double diff=fixAngleTwoSides(a-p.direction);
            if(diff > sett.kickableDirectionAngle){
                value+=outOfAngleDisadvantage;  
                outOfAngle=true; 
            }
            else if(diff < -sett.kickableDirectionAngle){
                value+=outOfAngleDisadvantage;   
                outOfAngle=true; 
            }
        } 
    }

    auto [towardsOwnGoal, towardsY] =isKickOrientedTowardOwnGoal(angle);
    if(towardsOwnGoal){
        if(outOfAngle){
            angle=bestKickAnglePossible(angle);
        }
        Vec2 ownGoal(pitch.x2, towardsY);
        double distGoal=(env.ball.pos-ownGoal).len();
        if( distGoal< ownGoalDistanceThreshold){
            double disadvantage= (ownGoalDistanceThreshold - distGoal)*ownGoalDistanceDisadvantage/ownGoalDistanceThreshold;
            value+=disadvantage;
        }
    }
    
    return value;
}

// value to pass to a companion in his still position
double SimplePlayer::companionPassValue(int player){
    return companionPassValue(player, false);
}

// value to pass to a companion in an advanced position
double SimplePlayer::companionPassValueAdvanced(int player){
    return companionPassValue(player, true);
}

std::tuple<Vec2, double> SimplePlayer::choosePass(){
    int nplayers=env.teams[0].size();
    
    double bestStill=-1000.0;
    int bestStillIndex=shirtNumber;
    double bestAdvanced=-1000.0;
    int bestAdvancedIndex=shirtNumber;
    for(int i=0; i<nplayers; i++){
        if(i!=shirtNumber){
            double valueStill=companionPassValue(i);
            if(bestStill<valueStill){
                bestStill=valueStill;
                bestStillIndex=i;
            }

            double valueAdvanced=companionPassValueAdvanced(i);
            if(bestAdvanced<valueAdvanced){
                bestAdvanced=valueAdvanced;
                bestAdvancedIndex=i;
            }

        }
    }

    if(bestStillIndex==shirtNumber){
        return std::tuple<Vec2, double>{Vec2(0.0, 0.0) , bestStill};
    }

    if(bestStill > bestAdvanced){
        return std::tuple<Vec2, double>{env.teams[0][bestStillIndex].pos , bestStill};
    }

    return std::tuple<Vec2, double>{calcAdvancedPosition(bestAdvancedIndex) , bestAdvanced};
}

R2Action SimplePlayer::chooseBestThrowin(){
    R2Action action;
    auto[pos, passValue]= choosePass();
    action= R2Action(R2ActionType::Kick, bestKickAnglePossible(ballAngleTowardPoint(pos.x,pos.y)), sett.maxKickPower);           
    return action;
}

// actually is the same as chooseBestThrowin(), but it can be improved for corner kick cases
R2Action SimplePlayer::chooseBestCornerKick(){
    R2Action action;
    auto[pos, passValue]= choosePass();
    action= R2Action(R2ActionType::Kick, bestKickAnglePossible(ballAngleTowardPoint(pos.x,pos.y)), sett.maxKickPower);   
    return action;
}

R2Action SimplePlayer::chooseBestAttackKick(){
    // check the best trajectory to kick the ball and do it
    int nIntervals=12;
    double bestY=0.0;
    double bestValue=0.0;
    for(int i=0; i<=nIntervals; i++){
        double py=pitch.yGoal1*(double(i)/nIntervals) + pitch.yGoal2*(double(nIntervals-i)/nIntervals);
        double distValue=minOpponentSegmentDistance(env.ball.pos, Vec2(pitch.x1, py));
        if(distValue>bestValue){
            bestValue=distValue;
            bestY=py;
        }
    }

    // not assured that actually you can kick that angle ! hence using bestKickAnglePossible()
    return R2Action(R2ActionType::Kick, bestKickAnglePossible(ballAngleTowardPoint(pitch.x1,bestY)), sett.maxKickPower);
}

constexpr double checkedDistance=3.0;
constexpr int nAdvancingIntervals=6;
constexpr double advancingIncentive=1.0;
constexpr double avoidOutIncentive=0.5;
constexpr double dangerBorderRatio=0.9;
constexpr double goalIncentive=50.0;
R2Action SimplePlayer::chooseBestPersonalBallAdvancing(){
    auto& p=env.teams[0][shirtNumber];
    double angleA=p.direction-sett.kickableAngle;
    double angleB=p.direction+sett.kickableAngle;
    double bestValue=0.0;
    double bestAngle=0.0;
    for(int i=0; i<=nAdvancingIntervals; i++){
        double angle=angleA*(double(i)/nAdvancingIntervals) + angleB*(double(nAdvancingIntervals-i)/nAdvancingIntervals);
        Vec2 target=Vec2(angle);
        target.resize(checkedDistance);
        target +=env.ball.pos;
        double value=minOpponentSegmentDistance(env.ball.pos, target);
        if(target.x > env.ball.pos.x)
            value+=advancingIncentive;
        double dangerBorder=pitch.y1*dangerBorderRatio;
        if(target.y > dangerBorder)
            value+=-(target.y-dangerBorder)/(pitch.y1*(1.0-dangerBorderRatio));
        else if(target.y < -dangerBorder)
            value+=(target.y-dangerBorder)/(pitch.y1*(1.0-dangerBorderRatio));
        double goalCloseness=goalIncentive*1.0/(1.0 + (Vec2(pitch.x1, 0.0)-target).len());
        double goalFactor=goalCloseness*goalCloseness;
        value+=goalFactor;

        auto [towardsOwnGoal, towardsY] =isKickOrientedTowardOwnGoal(angle);
        if(towardsOwnGoal){
            Vec2 ownGoal(pitch.x2, towardsY);
            double distGoal=(env.ball.pos-ownGoal).len();
            if( distGoal< ownGoalDistanceThreshold){
                double disadvantage= (ownGoalDistanceThreshold - distGoal)*ownGoalDistanceDisadvantage/ownGoalDistanceThreshold;
                value+=disadvantage;
            }
        }

        if(value>bestValue){
            bestValue=value;
            bestAngle=angle;
        }
    }
    return R2Action(R2ActionType::Kick, bestKickAnglePossible(bestAngle), sett.maxKickPower/2.0);
}

constexpr double minimumPassValue=0.1;
R2Action SimplePlayer::chooseBestPassOrAdvancing(){
    R2Action action;
    auto[pos, passValue]= choosePass();
    if(passValue>minimumPassValue){
        // pass the ball
        action= R2Action(R2ActionType::Kick, bestKickAnglePossible(ballAngleTowardPoint(pos.x,pos.y)), sett.maxKickPower);
    }
    else{
        action= chooseBestPersonalBallAdvancing();
    }
    return action;
}

constexpr double scoreDistance=8.0; // closer than this to goal => kick and try to score
R2Action SimplePlayer::play()
{
    R2Action action;

    if(isBallKickable()){
        if(prevState==R2State::Throwin1){
            action= chooseBestThrowin();
        }
        else if((prevState==R2State::Corner1down) || (prevState==R2State::Corner1up)){
            action= chooseBestCornerKick();
        }
        else{
            double goalDist=(env.ball.pos- Vec2(pitch.x1, 0.0)).len();
            if(goalDist<=scoreDistance){
                action= chooseBestAttackKick();
            }
            else{
                action=chooseBestPassOrAdvancing();
            }
        }
    }
    else{
        if(myPlayerClosestToBall()==shirtNumber){  // reach for the ball
            action= R2Action (R2ActionType::Dash, angleTowardPoint(env.ball.pos.x, env.ball.pos.y), sett.maxDashPower);
        }
        else{   // TODO decide if going back in defence, stay or advance. If out of the pitch, come back
            auto &pos=env.teams[0][shirtNumber].pos;
            if( pos.x > pitch.x1){
                if(pos.y < pitch.y2){   // out bottom right
                    action= R2Action (R2ActionType::Dash, M_PI*0.75, sett.maxDashPower);
                }
                else if(pos.y > pitch.y1){  // out top right
                    action= R2Action (R2ActionType::Dash, M_PI*1.25, sett.maxDashPower);
                }
                else{   //out right
                    action= R2Action (R2ActionType::Dash, M_PI, sett.maxDashPower);
                }
            }
            else if(pos.x < pitch.x2){
                if(pos.y < pitch.y2){   // out bottom left
                    action= R2Action (R2ActionType::Dash, M_PI*0.25, sett.maxDashPower);
                }
                else if(pos.y > pitch.y1){  // out top left
                    action= R2Action (R2ActionType::Dash, M_PI*1.75, sett.maxDashPower);
                }
                else{   //out left
                    action= R2Action (R2ActionType::Dash, 0.0, sett.maxDashPower);
                }
            }
            else if(pos.y > pitch.y1){  // out top
                action= R2Action (R2ActionType::Dash, M_PI*1.5, sett.maxDashPower);
            }
            else if(pos.y < pitch.y2){  // out bottom
                action= R2Action (R2ActionType::Dash, M_PI*0.5, sett.maxDashPower);
            }
            else{   // try to find your position
                if(shirtNumber!=0)
                    action= positionPlayer();
                else
                    action= positionGoalkeeper();
            }
        }
    }

    return action;
}

constexpr double markDistance=1.0;
R2Action SimplePlayer::positionPlayer(){
    const double goBackThresholdDistance=pitch.x2/2.0;
    const double goBackBaricenterThresholdDistance=pitch.x2/4.0;
    const double runForwardThresholdDistance=pitch.x1*2/3.0;

    R2Action action;

    if(isOpponentCloserToBall()){  //assume opponents are attacking

        if((env.ball.pos.x < goBackThresholdDistance) || (opponentBaricenter().dist(Vec2(pitch.x2,0.0))>goBackBaricenterThresholdDistance) ){
            // assign an opponent to mark. 0 is unassigned (you don't mark the goalkeepr), -1 is ball or no opponent. Goalkeeper is not marking, opponent goalkeeper is not marked
            std::vector<int> assignments(env.teams[0].size(), 0);
            int busy=myPlayerClosestToBall();    // this one is busy reaching for the ball
            assignments[busy]=-1;
            assignments[0]=-1;  //goalkeeper
            int busyOpponent=opponentPlayerClosestToBall();
            for(int i=1; i<env.teams[1].size(); i++){
                if(i!=busyOpponent){
                    int p=myPlayerClosestToOpponent(i, assignments);
                    if(p>0) // not assigning 0 (goal keeper) and -1 (no one)
                        assignments[p]=i;
                }
            }

            if(assignments[shirtNumber]< 0){   // not assigned (shouldn't be here if same number of players): reach for ball
                action= R2Action (R2ActionType::Dash, angleTowardBall(), sett.maxDashPower);
            }
            else if(assignments[shirtNumber] > 0){   // mark a player
                Vec2 target=env.teams[1][assignments[shirtNumber]].pos;
                Vec2 delta= Vec2(pitch.x2,0.0)-target;
                delta.resize(markDistance);
                target +=delta;
                action= R2Action (R2ActionType::Dash, angleTowardPoint(target), sett.maxDashPower);
            }

        }
        else{
            auto &pos=env.teams[0][shirtNumber].pos;
            double angle= (pos.y>0.0) ? angleTowardOwnUpperPole() : angleTowardOwnLowerPole();
            action= R2Action (R2ActionType::Dash, angle, sett.maxDashPower);
        }
    }
    else{
        auto &pos=env.teams[0][shirtNumber].pos;
        if(pos.x < runForwardThresholdDistance){
            double angle= (pos.y>0.0) ? angleTowardUpperPole() : angleTowardLowerPole();
            action= R2Action (R2ActionType::Dash, angle, sett.maxDashPower); 
        }
        else{   // try to stay in an unmarked position moving randomly
            double angle= uniformDist(rng)*2*M_PI;
            action= R2Action (R2ActionType::Dash, angle, sett.maxDashPower); 
        }
    }

    return action;
}

constexpr double goalkeeperInterventionDistance=3.0;
R2Action SimplePlayer::positionGoalkeeper(){
    R2Action action;
    auto& p= env.teams[0][shirtNumber];

    // if ball close, go for it
    if((p.pos.dist(env.ball.pos) < goalkeeperInterventionDistance) && (p.pos.x < 0.0) ){
        action= R2Action (R2ActionType::Dash, angleTowardBall(), sett.maxDashPower);
    }
    else{
        Vec2 goalCenter(pitch.x2, 0.0);
        Vec2 lineBallGoal=env.ball.pos-goalCenter;
        double wannaX= lineBallGoal.x/3.0;
        double wannaY= lineBallGoal.y/3.0;
        Vec2 wannaBe(pitch.x2+wannaX, wannaY);
        Vec2 posDiff=wannaBe-p.pos;

        if(posDiff.len()>0.5) {
            double angle=0.0;
            if(posDiff.x == 0.0){
                if(posDiff.y>0.0)
                    angle=M_PI_2;
                else
                    angle=-M_PI_2;
            }
            else {
                angle=atan2(posDiff.y, posDiff.x);
            }

            double dashPower=sett.maxDashPower;
            // to do: diminish dashPower when needed, to better adjust
            action= R2Action (R2ActionType::Dash, angle, dashPower);
        }
        else{   // if not moving, face the ball
            action= R2Action (R2ActionType::Dash, angleTowardBall(), 0.0);
        }
    }

    return action;
}

R2Action SimplePlayer::playGoalkeeper(){
    R2Action action;

    // 1) if ball is close and moving towards goal, catch it
    // 2) if ball is close and slow or still kick it to a companion (THE ONE THAT IS FAR FROM)
    // 3) otherwise goalkeeper wants to place himself on the line between the ball and the center of the goal
    //      choosing the distance depending on the most advanced opponent and on the ball
    
    auto p= env.teams[0][shirtNumber];
    auto [dist, d]=p.dist(env.ball); 

    double cosinusPlayerBall= (d.x*cos(p.direction) + d.y*sin(p.direction))/dist;

    if( (dist<sett.kickableDistance) &&
        (cosinusPlayerBall >= cosKickableAngle)  ){
        action= chooseBestPassOrAdvancing();    // actually if goalkeeper is holding the ball caught, this function is not accurate because the real ball position at the moment of kicking would not be the current in env (that is inside the goalkeeper) but instead it would be set right before the kick in front of the goalkeeper, at kick direction, at distance = sett.afterCatchDistance
    }
    else if( (dist< sett.catchableDistance ) &&    // is ball reachable ? then catch it
       ( (p.pos.x >= pitch.x2) && (p.pos.x <= pitch.areaLx) && (p.pos.y <= pitch.areaUy) && (p.pos.y >= pitch.areaDy) ) && // inside his area
       ( cosinusPlayerBall >= cosCatchableAngle)){ // -catchableAngle >= goalkeeper-ball angle <= catchableAngle 
       if(prevAction.action==R2ActionType::Kick){   
            //TODO: check if we need to catch it anyway
            return positionGoalkeeper();
       }
       action = R2Action(R2ActionType::Catch);
    }
    else {  // otherwise position yourself in the right way
       return positionGoalkeeper();
    }

    return action;
}


R2Action SimplePlayer::step(const R2GameState gameState) {
    sett=gameState.sett;
    cosKickableAngle=cos(sett.kickableAngle);
    cosCatchableAngle=cos(sett.catchableAngle);
    normalEnv=gameState.env;
    pitch=gameState.pitch;
    transformEnvIfNecessary();

    R2Action action;

    switch (env.state){
        case R2State::Inactive: 
            break;
        case R2State::Ready:  // currently, an unnecessary state
            break;
        case R2State::Kickoff1:{
            double x=-10.0,y=0.0;
            
            if (shirtNumber==0) {
                x=(pitch.x2+pitch.areaLx)/2;
                y=0.0;
            }
            else if(shirtNumber==1) {
                x=-sett.ballRadius-sett.playerRadius-R2Epsilon;
                y=0.0;
            }
            else if(shirtNumber==2) {
                x=-sett.ballRadius;
                y=sett.centerRadius;
            }
            else if(shirtNumber==3) {
                x=-pitch.x1/3;
                y=-sett.centerRadius;
            }
            else {   
                x=-pitch.x1+pitch.x1/(shirtNumber-1);
                y= sett.centerRadius*( (shirtNumber%3) -1);
            }
            
            action= R2Action(R2ActionType::Move, x, y);
            }
            break;
       
        case R2State::Kickoff2:{
            double x=-10.0,y=0.0;
            
            if (shirtNumber==0)
            {
                x=(pitch.x2+pitch.areaLx)/2;
                y=0.0;
            }
            else if(shirtNumber==1)
            {
                x=-sett.centerRadius-sett.playerRadius-R2Epsilon;
                y=0.0;
            }
            else if(shirtNumber==2)
            {
                x=pitch.x2/2;
                y=pitch.y2/2;
            }
            else if(shirtNumber==3)
            {
                x=pitch.x2/2;
                y=-pitch.y2/2;
            }
            else{
                x=-pitch.x1+pitch.x1/(shirtNumber-1);
                y= sett.centerRadius*( (shirtNumber%3) -1);
            }
        
            action= R2Action (R2ActionType::Move, x, y);
            }
            break;
        case R2State::Play:            
            if(shirtNumber==0)
                action= playGoalkeeper();
            else
                action= play();
            break;
        case R2State::Goalkick1up:  // make the goalkeeper kick
            if(shirtNumber==0){
                action= R2Action (R2ActionType::Move, pitch.goalKickLx-(sett.kickRadius/10+sett.playerRadius+sett.ballRadius), pitch.goalKickUy);
            }
            else {
                // look for the best place to stay
            }
            break;
        case R2State::Goalkick1down:
            if(shirtNumber==0){
                action= R2Action (R2ActionType::Move, pitch.goalKickLx-(sett.kickRadius/10+sett.playerRadius+sett.ballRadius), pitch.goalKickDy);
            }
            else {
                // look for the best place to stay
            }
            break;
        case R2State::Goalkick2up:  // reposition team
        case R2State::Goalkick2down:
            break;
        case R2State::Corner1up:    // the closest to the ball go there
            if(myPlayerClosestToBall()==shirtNumber){
                action= R2Action (R2ActionType::Move, pitch.x1, pitch.y1+(sett.kickRadius/10+sett.playerRadius+sett.ballRadius) );
            }
            break;
        case R2State::Corner1down:
            if(myPlayerClosestToBall()==shirtNumber){
                action= R2Action (R2ActionType::Move, pitch.x1, pitch.y2-(sett.kickRadius/10+sett.playerRadius+sett.ballRadius) );
            }
            break;
        case R2State::Corner2up:    // position close to the opponents
        case R2State::Corner2down:
            break;
        case R2State::Throwin1: // the closest to the ball go there
            if(myPlayerClosestToBall()==shirtNumber){
                double displace=sett.kickRadius/10+sett.playerRadius+sett.ballRadius;
                if(env.ball.pos.y<0.0)
                    displace*=-1;
                action= R2Action (R2ActionType::Move, env.ball.pos.x, env.ball.pos.y+displace);
            }
            break;
        case R2State::Throwin2: // position close to the opponents 
            break;
        case R2State::Paused:
            break;
        case R2State::Halftime:
            break;
        case R2State::Goal1:
        case R2State::Goal2:
            break;
        case R2State::Ended:
            break;
        default:
            break;
    }
    
    prevState = env.state;
    prevAction = transformActionIfNecessary(action);
    return prevAction;
}

std::unique_ptr<R2Simulator> buildSimplePlayerTwoTeamsSimulator(int nPlayers1, int nPlayers2, string _team1name, string _team2name, unsigned int random_seed, R2EnvSettings settings){ 
   return buildSimulator<SimplePlayer,SimplePlayer>(nPlayers1, nPlayers2, _team1name, _team2name, random_seed, settings);
}

std::unique_ptr<R2Simulator> buildSimplePlayerOneTeamSimulator(int nSimplePlayers, int simpleTeamNumber, std::vector<std::shared_ptr<R2Player>> otherTeam, string _team1name, string _team2name, unsigned int random_seed, R2EnvSettings settings){ 
    return buildOneTeamSimulator<SimplePlayer>(nSimplePlayers, simpleTeamNumber, otherTeam, _team1name, _team2name, random_seed, settings);
}

} // namespace