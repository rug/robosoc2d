// (c) 2021 Ruggero Rossi
// a simple, reactive, robosoc2d player agent (with no planning)
#ifndef R2S_SIMPLEPLAYER_H
#define R2S_SIMPLEPLAYER_H

#include "simulator.h"

namespace r2s {

class SimplePlayer : public R2Player {
private:
    int shirtNumber;
    int team;
    R2EnvSettings sett;
    R2Environment normalEnv;
    R2Environment env;  // to have the environmente always rotated as if your team is teams[0] on the left
    R2Pitch pitch;
    R2Action prevAction;
    R2State prevState;
    std::uniform_real_distribution<double> uniformDist;
    std::default_random_engine rng;
    double cosKickableAngle;
    double cosCatchableAngle;

    double minOpponentSegmentDistance(Vec2 s1, Vec2 s2);
    Vec2 calcAdvancedPosition(int player);
    double companionPassValue(int player, bool advanced);
    double companionPassValue(int player);
    double companionPassValueAdvanced(int player);
    std::tuple<Vec2, double> choosePass();
    void transformEnvIfNecessary();
    R2Action  transformActionIfNecessary(R2Action action);
    double bestKickAnglePossible(double angle);
    std::tuple<bool, double>  isKickOrientedTowardOwnGoal(double angle);
    double angleTowardPoint(double x, double y);
    double angleTowardPoint(Vec2 p) { return angleTowardPoint(p.x, p.y); }
    double angleTowardBall();
    double angleTowardCenterGoal();
    double angleTowardUpperPole();
    double angleTowardLowerPole();
    double angleTowardOwnUpperPole();
    double angleTowardOwnLowerPole();
    double ballAngleTowardPoint(double x, double y);
    double ballAngleTowardCompanion(int companion);
    double ballAngleTowardCenterGoal();
    double ballAngleTowardUpperPole();
    double ballAngleTowardLowerPole();
    bool isTeamResumeKicking();
    bool isBallKickable();
    bool isOpponentCloserToBall();
    Vec2 opponentBaricenter();

    int playerClosestToBall(std::vector<R2PlayerInfo> & team);
    int myPlayerClosestToBall();
    int opponentPlayerClosestToBall();
    int myClosestOpponent();
    int myClosestCompanion();
    int myPlayerClosestToOpponent(int opponent, std::vector<int> assigned);

    R2Action chooseBestThrowin();
    R2Action chooseBestCornerKick();
    R2Action chooseBestAttackKick();
    R2Action chooseBestPersonalBallAdvancing();
    R2Action chooseBestPassOrAdvancing();
    R2Action playStub();
    R2Action play();
    R2Action playGoalkeeper();
    R2Action positionPlayer();
    R2Action positionGoalkeeper();

public:
    SimplePlayer(int index, int _whichTeam=0);
    ~SimplePlayer(){}; 

    virtual R2Action step(const R2GameState gameState) override;
};

std::unique_ptr<R2Simulator> buildSimplePlayerTwoTeamsSimulator(int nPlayers1, int nPlayers2,
    std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name,
    unsigned int random_seed= createChronoRandomSeed(),
    R2EnvSettings settings = R2EnvSettings());

std::unique_ptr<R2Simulator> buildSimplePlayerOneTeamSimulator(int nSimplePlayers, int simpleTeamNumber, std::vector<std::shared_ptr<R2Player>> otherTeam,
    std::string team1name=defaultTeam1Name, std::string team2name=defaultTeam2Name,
    unsigned int random_seed= createChronoRandomSeed(),
    R2EnvSettings settings = R2EnvSettings()); 
} // end namespace

#endif // R2S_SIMPLEPLAYER_H