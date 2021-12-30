// (c) 2021 Ruggero Rossi
// robosoc2d : a Very Simplified 2D Robotic Soccer Simulator
#ifndef R2S_SIMULATOR_H
#define R2S_SIMULATOR_H

#include "vec2.h"

#ifdef _WIN32
    #include  <numeric>
    //#define __WXMSW__
    //#define _UNICODE
    //#define NDEBUG
#else // __linux__ 
#endif

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <random>
#include <array>
#include <set>
#include <limits>
#include <chrono>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
    #define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
    #define M_PI_4 0.785398163397448309616
#endif

namespace r2s { 

constexpr auto R2SVersion="1.0.0";
inline const char* GetR2SVersion(){return R2SVersion;};

constexpr auto defaultTeam1Name = "Team A";
constexpr auto defaultTeam2Name = "Team B";

constexpr double ContemporaryKickProbability=0.5;
constexpr double R2SmallEpsilon=std::numeric_limits<double>::epsilon();
constexpr double R2Epsilon=std::numeric_limits<double>::epsilon() * 10.0;
constexpr double R2BigEpsilon=std::numeric_limits<double>::epsilon() * 100.0;

constexpr double BallPlayerHitFactor=1.1;   //to avoid ball having exactly the same speed 
constexpr double BallPlayerHitFactorSimplified=1.001;   //to avoid ball having exactly the same speed (use 0.0 to have it completely stopped)
constexpr double BallPoleBounceFactor=0.95; // 0.95;
constexpr double BallPlayerBounceFactor=0.91;
constexpr double BallPlayerStopFactor=0.25;
constexpr double CollisionPlayerDisplaceFactor=0.2;

/*
constexpr double PlayerVelocityDecay=0.7; //0.7 big - 09 small;
constexpr double BallVelocityDecay=0.95; //0.95 big - 0.97 small;
constexpr double MaxPlayerSpeed=1.111; //1.111 big - 0.1111 small; // Bolt goes to more than 44 km/h. Let's say a good speed could be 40 km/h. It's 40m every 3,6 seconds, that's 11,111 m/s, that's 1,111 m/tick with 10 ticks per second.
constexpr double MaxBallSpeed=1.9; //1.9 big - 0.19 small; // 3 is almost 100 km/h
constexpr double MaxDashPower=0.2;//0.2 big - 0.02 small;
constexpr double MaxKickPower=1.5;//1.5 big - 0.15 small;
constexpr double KickRadius=0.2;//0.2 big - 0.1 small;
constexpr double BallRadius=0.3;    // 0.3 big - 0.11 small
constexpr double PlayerRadius=0.8;  // 0.8 big - 0.4 small
*/

constexpr double PlayerVelocityDecay=0.9; //0.7 big - 09 small;
constexpr double BallVelocityDecay=0.97; //0.95 big - 0.97 small;
constexpr double MaxPlayerSpeed=0.2222; //1.111 big - 0.1111 small; // Bolt goes to more than 44 km/h. Let's say a good speed could be 40 km/h. It's 40m every 3,6 seconds, that's 11,111 m/s, that's 1,111 m/tick with 10 ticks per second.
constexpr double MaxBallSpeed=0.6; //1.9 big - 0.19 small; // 3 is almost 100 km/h
constexpr double MaxDashPower=0.06;//0.2 big - 0.02 small;
constexpr double MaxKickPower=0.65;//1.5 big - 0.15 small; // old defaul 0.45
constexpr double KickRadius=0.1;//0.2 big (old standard) - 0.1 small; in addition to player radius
constexpr double BallRadius=0.11;    // 0.3 big - 0.11 small
constexpr double PlayerRadius=0.4;  // 0.8 big - 0.4 small

constexpr double PlayerRandomNoise=0.005;//=0.2;
constexpr double PlayerDirectionNoise=0.005;//=0.2;
constexpr double PlayerVelocityDirectionMix=0.2;//=0.2;
constexpr double BallInsidePlayerVelocityDisplace=0.5;//=0.2;
constexpr double AfterCatchDistance=0.05;
constexpr int   CatchHoldingTicks=2;
constexpr double PoleRadius=0.055; 

constexpr double CatchProbability=0.9;
constexpr double CatchRadius=0.3;
constexpr double KickableAngle= M_PI/3.0; //kickable angle between player direction and ball-player vector
constexpr double KickableDirectionAngle= M_PI_2; //kickable angle between player direction and direction of the kick
constexpr double CatchableAngle= M_PI_2; //
constexpr double CosCatchableAngle=0.0; // M_PI/3

constexpr double RegularPitchLength=105.0;
constexpr double RegularPitchWidth=68.0;
constexpr double RegularAreaLength=16.5;
constexpr double RegularAreaWidth=40.32;
constexpr double RegularGoalWidth=7.32;
constexpr double RegularCornerDistance=9.15;    // minimum distance of opponent when kicking corner
constexpr double MinimumCornerDistance=2.5; // this is typical for 5-a-side soccer = 4.0
constexpr double MinimumThrowinDistance=2.5; // this is typical for 5-a-side soccer = 4.0
constexpr int   MaxCollisionLoop=10;    // 10
constexpr int   MaxCollisionInsideTickLoop=40;  //40
constexpr double PlayerOutOfPitchLimit=3.0;

inline double calcAreaLength(double pitchLength) { return pitchLength/RegularPitchLength*RegularAreaLength; }
inline double calcAreaWidth(double pitchWidth) { return pitchWidth/RegularPitchWidth*RegularAreaWidth; }
inline double calcCornerDistance(double pitchWidth) { double d=pitchWidth/RegularPitchWidth*RegularCornerDistance; return (d<MinimumCornerDistance)? MinimumCornerDistance : d; }

inline unsigned int createChronoRandomSeed(){ return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); } 

//sets angle between 0 and 2 PI
double fixAnglePositive(double angle);
//sets angle between + and - PI
double fixAngleTwoSides(double angle);

struct R2Pitch {
    double x1,x2,y1,y2;
    double xGoal1, xGoal2;
    double yGoal1, yGoal2;
    double areaLx, areaRx, areaUy, areaDy;
    double goalKickLx, goalKickRx, goalKickUy, goalKickDy;
    Vec2 poles[4];
    double border_up, border_down, border_left, border_right;

    R2Pitch(double _pitchWidth=RegularPitchWidth, double _pitchLength=RegularPitchLength, double _goalWidth=RegularGoalWidth,
     double _netLength=0.0, double _poleRadius=PoleRadius, double _borderLimit=PlayerOutOfPitchLimit):
        x1(_pitchLength/2.0), x2(-x1),
        y1(_pitchWidth/2.0), y2(-y1), 
        xGoal1(x1+_netLength), xGoal2( -xGoal1 ),
        yGoal1(_goalWidth/2.0), yGoal2( -yGoal1 ),
        areaLx(x2+calcAreaLength(_pitchLength)), 
        areaRx(x1-calcAreaLength(_pitchLength)), 
        areaUy(calcAreaWidth(_pitchWidth)/2.0),
        areaDy(-calcAreaWidth(_pitchWidth)/2.0), 
        goalKickLx(x2+(areaLx-x2)/2.0),  
        goalKickRx(x1+(areaRx-x1)/2.0),
        goalKickUy(yGoal1+(areaUy-yGoal1)/2.0),
        goalKickDy(yGoal2+(areaDy-yGoal2)/2.0),
        poles{Vec2(x2, yGoal1+_poleRadius), Vec2(x2, yGoal2-_poleRadius), Vec2(x1, yGoal1+_poleRadius), Vec2(x1, yGoal2-_poleRadius)},
        border_up(y1+_borderLimit), border_down(y2-_borderLimit), border_left(x2-_borderLimit), border_right(x1+_borderLimit)  {}
};
struct R2EnvSettings {
    bool simplified;
    int ticksPerTime;
    double pitchLength;
    double pitchWidth;
    double goalWidth;
    double centerRadius;
    double poleRadius;   
    double ballRadius;
    double playerRadius;
    double catchRadius;
    int catchHoldingTicks;
    double kickRadius;
    double kickableDistance;
    double catchableDistance;
    double kickableAngle;
    double kickableDirectionAngle;
    double catchableAngle;
    double netLength;
    double catchableAreaLength;
    double catchableAreaWidth;
    double cornerMinDistance;
    double throwinMinDistance;
    double outPitchLimit;
    double maxDashPower;
    double maxKickPower;
    double playerVelocityDecay;
    double ballVelocityDecay;
    double maxPlayerSpeed;
    double maxBallSpeed;
    double catchProbability;
    double playerRandomNoise;
    double playerDirectionNoise;
    double playerVelocityDirectionMix;
    double ballInsidePlayerVelocityDisplace;
    double afterCatchDistance;

    /**
        with 11 players: 105x68 with 7.32 mts goal, area 16,5x40,32, central circle 9,15m
        with 5 players: 40x24 with 4 mts goal, center circle 3m radius, area (proportion) 
        with 4 players: 32x19.2 with 4 mts goal, center circle 2,4m radius
    */
    R2EnvSettings(bool _simplified=true, double _pitchLength=32, double _pitchWidth=19.2, double _goalWidth=4.0, double _centerRadius=2.4,
    int _ticksPerTime=3000, double _poleRadius=PoleRadius,  double _ballRadius=BallRadius, double _playerRadius=PlayerRadius, double _catchRadius=CatchRadius, double _kickRadius=KickRadius,
    double _kickableAngle=KickableAngle, double _kickableDirectionAngle=KickableDirectionAngle, double _catchableAngle=CatchableAngle,  double _netLength=1.5) :
        simplified(_simplified), ticksPerTime(_ticksPerTime), pitchLength(_pitchLength), pitchWidth(_pitchWidth), goalWidth(_goalWidth), 
        centerRadius(_centerRadius), poleRadius(_poleRadius), ballRadius(_ballRadius), playerRadius(_playerRadius), catchRadius(_catchRadius), catchHoldingTicks(CatchHoldingTicks),
        kickRadius(_kickRadius), kickableDistance(kickRadius+playerRadius+ballRadius), catchableDistance(catchRadius+playerRadius+ballRadius),
        kickableAngle(_kickableAngle), kickableDirectionAngle(_kickableDirectionAngle),
        catchableAngle(_catchableAngle), netLength(_netLength),
        catchableAreaLength(calcAreaLength(_pitchLength)), catchableAreaWidth(calcAreaWidth(_pitchWidth)),
        cornerMinDistance(calcCornerDistance(_pitchWidth)), throwinMinDistance(MinimumThrowinDistance), outPitchLimit(PlayerOutOfPitchLimit),
        maxDashPower(MaxDashPower), maxKickPower(MaxKickPower),
        playerVelocityDecay(PlayerVelocityDecay), ballVelocityDecay(BallVelocityDecay), maxPlayerSpeed(MaxPlayerSpeed), maxBallSpeed(MaxBallSpeed),
        catchProbability(CatchProbability), playerRandomNoise(PlayerRandomNoise), playerDirectionNoise(PlayerDirectionNoise), playerVelocityDirectionMix(PlayerVelocityDirectionMix),
        ballInsidePlayerVelocityDisplace(BallInsidePlayerVelocityDisplace), afterCatchDistance(AfterCatchDistance)   {}    
};

enum class R2State {
    Inactive, //!< not active at all
    Ready,    //!< UNUSED 
    Kickoff1,  //!< kickoff (beginning or after a goal), team 1 (left)
    Kickoff2,
    Play,     //!< game is playing
    Stopped,  //!< UNUSED - game is stopped
    Goalkick1up,   //!< goal-kick team 1 (left)
    Goalkick1down,   //!< goal-kick team 1 (left)
    Goalkick2up,   //!< goal-kick
    Goalkick2down,   //!< goal-kick
    Corner1up,
    Corner1down,
    Corner2up,
    Corner2down,
    Throwin1,  //throw-in
    Throwin2,  
    Paused,   //!< UNUSED - simulation paused
    Halftime, //!< half time interval (if any, not sure it ever reaches this state)
    Goal1,     //!< Team1 scored a goal
    Goal2,     //!< Team2 scored a goal
    Ended     //!< end of the match
};

struct R2ObjectInfo {
    Vec2 pos, velocity;

    R2ObjectInfo(double _x=0.0, double _y=0.0, double _xVelocity=0.0, double _yVelocity=0.0):
        pos(_x, _y), velocity(_xVelocity, _yVelocity) {}

    double absVelocity() { return velocity.len(); };
    double absDistanceFromCenter() { return pos.len(); };

    std::pair<double, Vec2> dist(R2ObjectInfo& obj){
        Vec2 d = obj.pos - pos;
        return std::pair<double, Vec2> { d.len(), d};
    }
};

struct R2PlayerInfo : R2ObjectInfo {
    double direction; //<! where's pointing the front of the player. In radiants. 0.0 points towards right of the field.
    bool acted; //if it already acted during this tick
    R2PlayerInfo(double _x=0.0, double _y=0.0, double _xVelocity=0.0, double _yVelocity=0.0, double _direction=0.0) : R2ObjectInfo(_x, _y, _xVelocity, _yVelocity) , direction(_direction), acted(false) {}
};

struct R2Environment {
    int tick;    //!<current tick
    int score1;
    int score2;
    R2State state;
    R2ObjectInfo ball;
    std::vector<R2PlayerInfo> teams[2];

    bool lastTouchedTeam2;  
    double startingTeamMaxRange;    // max movement done by the team that is starting the kick (goal kick, throwin, corner)
    int ballCatched;        // if >0 the ball is catched and only the goalkeeper can kick it and the ball moves with him. The value indicates how many ticks long the goalkeeper can hold the ball.
    bool ballCatchedTeam2;  // if ballis catched, this says if its catched by Team2 goalkeeper
    bool halftimePassed;

    R2Environment(int nPlayers1=0, int nPlayers2=0) :
        tick(0), score1(0), score2(0), state(R2State::Inactive), ball(), 
        teams{std::vector<R2PlayerInfo>(nPlayers1),std::vector<R2PlayerInfo>(nPlayers2)},
        lastTouchedTeam2(false), startingTeamMaxRange(0.0), ballCatched(0), ballCatchedTeam2 (false), halftimePassed(false) {}
};

struct R2GameState {
    R2EnvSettings sett;
    R2Environment env;
    R2Pitch pitch;
    R2GameState(R2EnvSettings _sett, R2Environment _env, R2Pitch _pitch) : sett(_sett), env(_env), pitch(_pitch) {}
};

enum class R2ActionType {
    NoOp,   //<! don't do anything
    Move,   //<! position on the pitch
    Dash,   //<! accelerate
    Turn,   //<! UNUSED
    Kick,
    Catch   //<! only goalkeeper, in area
};

struct R2Action {
    R2ActionType action;
    double data[3];

    R2Action(R2ActionType actionType=R2ActionType::NoOp, double data1=0.0, double data2=0.0, double data3=0.0):
        action(actionType), data{data1, data2, data3} {}
};

class R2Player {
public:
    virtual R2Action step(const R2GameState gameState) = 0;
    virtual ~R2Player() = default; 
};

std::tuple<int, double, double> intersectionSegmentCircle(Vec2 s1, Vec2 s2, Vec2 c1, double r);

struct R2ActionRecord{
    int team;
    int player;
    R2Action action;

    R2ActionRecord(int _team=0, int _player=0, R2Action _action=R2Action()):
        team(_team), player(_player), action(_action) {}
};

struct R2History{
std::vector<R2Environment> envs;
std::vector<std::vector<R2ActionRecord>> actions;  // is team 2, player, action

R2History(int ticks, int nplayers1, int nplayers2) : envs(ticks+1), actions(ticks, std::vector<R2ActionRecord>(nplayers1+nplayers2) ){};
};

struct R2BallPlayerCollision{
    double t;
    int p;
    int team;
    R2BallPlayerCollision(double _t, int _p, int _team) : 
        t(_t), p(_p), team(_team) {}
};

struct R2PoleBallCollision{
    bool collision;
    double t;
    int pole;   
    R2PoleBallCollision(bool _collision, double _t, int _pole) : 
        collision(_collision), t(_t), pole(_pole) {}
};

struct R2PolePlayerCollision{
    double t;
    int p;
    int team;
    int pole;
    R2PolePlayerCollision(double _t, int _p, int _team, int _pole) : 
        t(_t), p(_p), team(_team), pole(_pole) {}
};

struct R2PlayerPlayerCollision{
    double t;
    int p1;
    int team1;
    int p2;
    int team2;

    R2PlayerPlayerCollision(double _t, int _p1, int _team1, int _p2, int _team2) : 
        t(_t), p1(_p1), team1(_team1), p2(_p2), team2(_team2) {}
};

enum class R2CollisionType {
    None,
    PoleBall,
    PolePlayer,
    BallPlayer,
    PlayerPlayer
};

struct R2CollisionTime{
    double t;
    R2CollisionType type;
    R2CollisionTime(double _t, R2CollisionType _type): t(_t), type(_type){}
};

class R2Simulator{
private:
        R2EnvSettings sett;
        R2Environment env, oldEnv;
        R2Pitch pitch;
        unsigned int random_seed;
        std::default_random_engine rng;
        std::normal_distribution<double> normalDist;
        std::uniform_real_distribution<double> uniformDist;
        std::vector<std::shared_ptr<R2Player>> teams[2];
        std::vector<int> shuffledPlayers;
        bool startedTeam2;
        bool ballAlreadyKicked;
        const std::set<R2State> notStarterStates;
        const std::set<R2State> team2StarterStates;
        R2History history;
        int processedActions;
        std::string teamNames[2];
        double cosKickableAngle;
        double cosCatchableAngle;

        bool isBallOutUp(){return (env.ball.pos.y > pitch.y1);}
        bool isBallOutDown(){return (env.ball.pos.y < pitch.y2);}
        bool isBallOutLeft(){return (env.ball.pos.x < pitch.x2);}
        bool isBallOutRight(){return (env.ball.pos.x > pitch.x1);}
        bool isBallOut(){return (isBallOutUp()||isBallOutDown()||isBallOutLeft()||isBallOutRight());}
        
        void resetPlayersActed();
        void setBallThrowInPosition();
        bool isBallInGoal(const int team);  // Team is the one scoring
        bool isGoalScored(const int team);
        bool didBallIntersectGoalLine(const int team);
        bool isPlayerOut(const int player, const int team);
        bool isPlayerInsideHisArea(const int player, const int team);
        bool isPlayerInsideOpponentArea(const int player, const int team);
        bool isPlayerInsideOpponentAreaFullBody(Vec2 pos, const int team);
        bool isPlayerInsideOpponentAreaFullBody(const int player, const int team);
        void limitBallSpeed();
        void limitPlayerSpeed(R2PlayerInfo& p);
        void limitSpeed();
        void decayPlayerSpeed(R2PlayerInfo& p);
        void decaySpeed();
        void putPlayersFarFromBall(int team, double minDist);
        Vec2 avoidOtherPlayersPosition(Vec2 pos, int team, int player);
        void actionMove(const R2Action& action, int team, int player);
        void actionMoveKickoff(const R2Action& action, int team, int player);
        void actionMoveGoalkick(const R2Action& action, int team, int player);
        void actionMoveThrowinCorner(const R2Action& action, int team, int player, double distanceToBall);
        void actionMoveThrowin(const R2Action& action, int team, int player);
        void actionMoveCorner(const R2Action& action, int team, int player);
        void actionDash(const R2Action& action, int team, int player);
        void actionKick(const R2Action& action, int team, int player);
        void actionCatch(const R2Action& action, int team, int player);
        void setBallCatchedPosition();
        void setBallReleasedPosition();
        void limitPlayersCloseToPitch();
        void limitPlayersToHalfPitch(int kickTeam, double dueDistance);
        void limitPlayersToHalfPitchCenterBody(int kickTeam){limitPlayersToHalfPitch(kickTeam, sett.centerRadius);}
        void limitPlayersToHalfPitchFullBody(int kickTeam){limitPlayersToHalfPitch(kickTeam, sett.centerRadius+sett.playerRadius);}
        void limitPlayersOutsideArea(int kickTeam);
        void limitPlayersOutsideAreaFullBody(int kickTeam);
        bool checkBallOut();
        bool checkGoalOrBallOut();
        void processStep(const R2Action& action, const int team, const int player);
        void procReady(const R2Action& action, const int team, const int player);
        void procKickoff(const R2Action& action, const int team, const int player);
        void procPlay(const R2Action& action, const int team, const int player);
        void procGoalkick(const R2Action& action, const int team, const int player);
        void procCorner(const R2Action& action, const int team, const int player);
        void procThrowin(const R2Action& action, const int team, const int player);
        void procGoal(const R2Action& action, const int team, const int player);
        void procEnded(const R2Action& action, const int team, const int player);
        void preState();
        void checkState();
        std::tuple<bool, double> findPoleObjectCollision(R2ObjectInfo& obj1, Vec2 pole, double radius, double partialT);
        std::tuple<bool, double> findObjectsCollision(R2ObjectInfo& obj1, R2ObjectInfo&obj2, double radius, double partialT);

        std::tuple<bool, double> findBallPlayerCollision(int team, int player, double partialT);
        std::vector<R2BallPlayerCollision> findFirstBallPlayersCollisions(double partialT, std::vector<bool>& ballPlayerBlacklist);
        std::tuple<bool, double> findPlayerPlayerCollision(int team1, int player1, int team2, int player2, double partialT);
        std::vector<R2PlayerPlayerCollision> findFirstPlayerPlayersCollisions(double partialT, std::vector<int>& playerPlayerCollisions);
       
        std::tuple<bool, double> findPoleBallCollision(Vec2 pole, double partialT);
        std::tuple<bool, double> findPolePlayerCollision(int team, int player, Vec2 pole, double partialT);

        R2PoleBallCollision findFirstPoleBallCollision(double partialT);
        std::vector<R2PolePlayerCollision> findFirstPolePlayersCollisions(double partialT);


        void addBallNoise();
        void updateMotion(double t);
        void manageCollisions();
        bool manageStaticPoleBallCollisions();
        void manageStaticBallCollisions();
        bool manageStaticPolePlayersCollisions();
        void manageStaticPlayersCollisions();
        void updateCollisionsAndMovements();
        bool isAnyTeamPreparingKicking();
        bool isAnyTeamKicking();
        bool isTeam2Kicking(R2State theState);
        void playersAct();

        void manageBallInsidePlayers();
public:
    R2Simulator(std::vector<std::shared_ptr<R2Player>> _team1, std::vector<std::shared_ptr<R2Player>> _team2, std::string _team1name=defaultTeam1Name, std::string _team2name=defaultTeam2Name,
      unsigned int _random_seed = createChronoRandomSeed() ,
      R2EnvSettings _settings = R2EnvSettings() ) :
        sett(_settings) , env(_team1.size(), _team2.size()) ,  oldEnv(_team1.size(), _team2.size()),
        pitch(_settings.pitchWidth, _settings.pitchLength, _settings.goalWidth, _settings.netLength, _settings.poleRadius, _settings.outPitchLimit),
        random_seed (_random_seed),
        rng (_random_seed),
        normalDist(), uniformDist(0.0, 1.0),
        teams{_team1, _team2} ,
        shuffledPlayers(_team1.size()+_team2.size(),0),
        startedTeam2(false),
        ballAlreadyKicked(false),
        notStarterStates({R2State::Inactive, R2State::Ready, R2State::Play, R2State::Stopped, R2State::Paused, R2State::Halftime, R2State::Goal1, R2State::Goal2, R2State::Ended}),
        team2StarterStates({R2State::Kickoff2, R2State::Goalkick2up, R2State::Goalkick2down, R2State::Corner2up, R2State::Corner2down, R2State::Throwin2}),
        history(_settings.ticksPerTime*2, _team1.size(), _team2.size()),
        processedActions(0),
        teamNames{_team1name ,_team2name},
        cosKickableAngle (cos(sett.kickableAngle)),
        cosCatchableAngle(cos(sett.catchableAngle))
        {
            std::iota (std::begin(shuffledPlayers), std::end(shuffledPlayers), 0);
        }

    void setStartMatch();
    void setHalfTime();
    void playMatch();
    void step();
    bool stepIfPlaying();
    R2GameState getGameState() { return R2GameState(sett, env, pitch); };
    std::vector<std::string> getTeamNames();
    std::string getStateString();
    unsigned int getRandomSeed() { return random_seed;};
    std::string createDateFilename();
    bool saveStatesHistory(std::string filename);
    bool saveStatesHistory(){ return saveStatesHistory(createDateFilename().append(".states.txt")); }
    bool saveActionsHistory(std::string filename);
    bool saveActionsHistory(){ return saveActionsHistory(createDateFilename().append(".actions.txt")); }
    bool saveHistory(){ std::string fn1,fn2=createDateFilename(); fn1=fn2; bool r=saveStatesHistory(fn1.append(".states.txt")); return ( saveActionsHistory(fn2.append(".actions.txt")) && r);}

    void setEnvironment(int _tick, int _score1, int _score2, R2State _state, R2ObjectInfo _ball, 
        std::vector<R2PlayerInfo> _team1, std::vector<R2PlayerInfo> _team2,
        bool _lastTouchedTeam2, int _ballCatched, bool _ballCatchedTeam2) ;
};

template <typename player> 
std::vector<std::shared_ptr<R2Player>> buildTeam(int nPlayers, int whichTeam){
    std::vector<std::shared_ptr<R2Player>> team;
    static_assert(std::is_base_of<R2Player, player>::value, "Class of player not derived from R2Player");
    for (int i=0 ; i <nPlayers; i++){
        team.push_back(std::static_pointer_cast<R2Player>(std::make_shared<player>(i, whichTeam)));
    }
    return team;
}

template <typename goalkeeper, typename players> 
std::vector<std::shared_ptr<R2Player>> buildTeam(int nPlayers, int whichTeam){
    std::vector<std::shared_ptr<R2Player>> team;
    static_assert(std::is_base_of<R2Player, goalkeeper>::value, "Class of goalkeeper not derived from R2Player");
    static_assert(std::is_base_of<R2Player, players>::value, "Class of player not derived from R2Player");

    if(nPlayers>0)
        team.push_back(std::static_pointer_cast<R2Player>(std::make_shared<goalkeeper>(0, whichTeam)));

    for (int i=1 ; i <nPlayers; i++){
        team.push_back(std::static_pointer_cast<R2Player>(std::make_shared<players>(i, whichTeam)));
    }
    return team;
}

template<typename team1Player, typename team2Player>
std::unique_ptr<R2Simulator> buildSimulator(int nPlayers1, int nPlayers2, std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name, 
        unsigned int random_seed = createChronoRandomSeed() ,
        R2EnvSettings settings = R2EnvSettings() ){
    std::vector<std::shared_ptr<R2Player>> team1, team2;
    team1= buildTeam<team1Player>(nPlayers1,0);
    team2= buildTeam<team2Player>(nPlayers2,1);
    return std::make_unique<R2Simulator>(team1, team2, team1name, team2name, random_seed, settings);
}

template<typename team1Goalkeeper, typename team1Player, typename team2goalkeeper, typename team2Player>
std::unique_ptr<R2Simulator> buildSimulator(int nPlayers1, int nPlayers2, std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name, 
        unsigned int random_seed = createChronoRandomSeed() ,
        R2EnvSettings settings = R2EnvSettings()){
    std::vector<std::shared_ptr<R2Player>> team1, team2;
    team1= buildTeam<team1Goalkeeper,team1Player>(nPlayers1,0);
    team2= buildTeam<team2goalkeeper,team2Player>(nPlayers2,1);
    return std::make_unique<R2Simulator>(team1, team2, team1name, team2name, random_seed, settings);
}

template<typename player>
std::unique_ptr<R2Simulator> buildOneTeamSimulator(int nPlayers, int teamNumber, std::vector<std::shared_ptr<R2Player>> otherTeam,
        std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name, 
        unsigned int random_seed = createChronoRandomSeed() ,
        R2EnvSettings settings = R2EnvSettings()){ 
    std::vector<std::shared_ptr<R2Player>>  team= buildTeam<player>(nPlayers, teamNumber);
    return teamNumber ? std::make_unique<R2Simulator>(otherTeam, team, team1name, team2name, random_seed, settings) : std::make_unique<R2Simulator>(team, otherTeam, team1name, team2name, random_seed, settings);
}

template<typename goalkeeper, typename player>
std::unique_ptr<R2Simulator> buildOneTeamSimulator(int nPlayers, int teamNumber, std::vector<std::shared_ptr<R2Player>> otherTeam,
    std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name, 
        unsigned int random_seed = createChronoRandomSeed() ,
    R2EnvSettings settings = R2EnvSettings()){ 
    std::vector<std::shared_ptr<R2Player>>  team= buildTeam<goalkeeper, player>(nPlayers, teamNumber);
    return teamNumber ? std::make_unique<R2Simulator>(otherTeam, team, team1name, team2name, random_seed, settings) : std::make_unique<R2Simulator>(team, otherTeam, team1name, team2name, random_seed, settings);
}

} // end namespace
#endif // SIMULATOR_H