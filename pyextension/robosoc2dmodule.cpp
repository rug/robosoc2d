// (c) 2021 Ruggero Rossi
// to build:
//  python3 setup.py build
// to install:
//  python3 setup.py install
// to uninstall it
//  pip uninstall robosoc2d

//#define _R2S_DEBUG

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "../src/simulator.cpp"
#include "../src/simple_player.cpp"
#include "../src/debug_print.h"

using namespace std;
using namespace r2s;

#include <chrono>
//using namespace std::chrono;
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>

constexpr auto defaultPythonTeam1Name = "Python Team A";
constexpr auto defaultPythonTeam2Name = "Python Team B";

static bool isPythonActive=false;
static PyObject *R2Error;

struct R2SettingsObject{
    PyObject_HEAD
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
};

static PyObject *R2SettingsObject_repr(R2SettingsObject * obj){
    std::stringstream buffer;
    buffer << "{" 
        << "'simplified': " << obj->simplified << ", "
        << "'ticks_per_time': " << obj->ticksPerTime << ", "
        << "'pitch_length': " << obj->pitchLength << ", "
        << "'pitch_width': " << obj->pitchWidth << ", "
        << "'goal_width': " << obj->goalWidth << ", "
        << "'center_radius': " << obj->centerRadius << ", "
        << "'pole_radius': " << obj->poleRadius << ", "
        << "'ball_radius': " << obj->ballRadius << ", "
        << "'player_radius': " << obj->playerRadius << ", "
        << "'catch_radius': " << obj->catchRadius << ", "
        << "'catch_holding_ticks': " << obj->catchHoldingTicks << ", "
        << "'kick_radius': " << obj->kickRadius << ", "
        << "'kickable_distance': " << obj->kickableDistance << ", "
        << "'catchable_distance': " << obj->catchableDistance << ", "
        << "'kickable_angle': " << obj->kickableAngle << ", "
        << "'kickable_direction_angle': " << obj->kickableDirectionAngle << ", "
        << "'catchable_angle': " << obj->catchableAngle << ", "
        << "'net_length': " << obj->netLength << ", "
        << "'catchable_area_length': " << obj->catchableAreaLength << ", "
        << "'catchable_area_width': " << obj->catchableAreaWidth << ", "
        << "'corner_min_distance': " << obj->cornerMinDistance << ", "
        << "'throwin_min_distance': " << obj->throwinMinDistance << ", "
        << "'out_pitch_limit': " << obj->outPitchLimit << ", "
        << "'max_dash_power': " << obj->maxDashPower << ", "
        << "'max_kick_power': " << obj->maxKickPower << ", "
        << "'player_velocity_decay': " << obj->playerVelocityDecay << ", "
        << "'ball_velocity_decay': " << obj->ballVelocityDecay << ", "
        << "'max_player_speed': " << obj->maxPlayerSpeed << ", "
        << "'max_ball_speed': " << obj->maxBallSpeed << ", "
        << "'catch_probability': " << obj->catchProbability << ", "
        << "'player_random_noise': " << obj->playerRandomNoise << ", "
        << "'player_direction_noise': " << obj->playerDirectionNoise << ", "
        << "'player_velocity_direction_mix': " << obj->playerVelocityDirectionMix << ", "
        << "'ball_inside_player_velocity_displace': " << obj->ballInsidePlayerVelocityDisplace << ", "
        << "'after_catch_distance': " << obj->afterCatchDistance
        << "}" ;
    return PyUnicode_FromString(buffer.str().c_str());
}

static void
R2SettingsType_dealloc(R2SettingsObject * obj)
{
    Py_TYPE(obj)->tp_free((PyObject *) obj);
}

void fillR2SettingsObject(R2SettingsObject& target, const R2EnvSettings& source){
    target.simplified = source.simplified;
    target.ticksPerTime = source.ticksPerTime;
    target.pitchLength = source.pitchLength;
    target.pitchWidth = source.pitchWidth;
    target.goalWidth = source.goalWidth;
    target.centerRadius = source.centerRadius;
    target.poleRadius = source.poleRadius;   
    target.ballRadius = source.ballRadius;
    target.playerRadius = source.playerRadius;
    target.catchRadius = source.catchRadius;
    target.catchHoldingTicks = source.catchHoldingTicks;
    target.kickRadius = source.kickRadius;
    target.kickableDistance = source.kickableDistance;
    target.catchableDistance = source.catchableDistance;
    target.kickableAngle = source.kickableAngle;
    target.kickableDirectionAngle = source.kickableDirectionAngle;
    target.catchableAngle = source.catchableAngle;
    target.netLength = source.netLength;
    target.catchableAreaLength = source.catchableAreaLength;
    target.catchableAreaWidth = source.catchableAreaWidth;
    target.cornerMinDistance = source.cornerMinDistance;
    target.throwinMinDistance = source.throwinMinDistance;
    target.outPitchLimit = source.outPitchLimit;
    target.maxDashPower = source.maxDashPower;
    target.maxKickPower = source.maxKickPower;
    target.playerVelocityDecay = source.playerVelocityDecay;
    target.ballVelocityDecay = source.ballVelocityDecay;
    target.maxPlayerSpeed = source.maxPlayerSpeed;
    target.maxBallSpeed = source.maxBallSpeed;
    target.catchProbability = source.catchProbability;
    target.playerRandomNoise = source.playerRandomNoise;
    target.playerDirectionNoise = source.playerDirectionNoise;
    target.playerVelocityDirectionMix = source.playerVelocityDirectionMix;
    target.ballInsidePlayerVelocityDisplace = source.ballInsidePlayerVelocityDisplace;
    target.afterCatchDistance = source.afterCatchDistance;
}

// filling C++ settings from Python settings (the inverse of the above)
void fillR2Settings(R2EnvSettings& target, const R2SettingsObject& source){
    target.simplified = source.simplified;
    target.ticksPerTime = source.ticksPerTime;
    target.pitchLength = source.pitchLength;
    target.pitchWidth = source.pitchWidth;
    target.goalWidth = source.goalWidth;
    target.centerRadius = source.centerRadius;
    target.poleRadius = source.poleRadius;   
    target.ballRadius = source.ballRadius;
    target.playerRadius = source.playerRadius;
    target.catchRadius = source.catchRadius;
    target.catchHoldingTicks = source.catchHoldingTicks;
    target.kickRadius = source.kickRadius;
    target.kickableDistance = source.kickableDistance;
    target.catchableDistance = source.catchableDistance;
    target.kickableAngle = source.kickableAngle;
    target.kickableDirectionAngle = source.kickableDirectionAngle;
    target.catchableAngle = source.catchableAngle;
    target.netLength = source.netLength;
    target.catchableAreaLength = source.catchableAreaLength;
    target.catchableAreaWidth = source.catchableAreaWidth;
    target.cornerMinDistance = source.cornerMinDistance;
    target.throwinMinDistance = source.throwinMinDistance;
    target.outPitchLimit = source.outPitchLimit;
    target.maxDashPower = source.maxDashPower;
    target.maxKickPower = source.maxKickPower;
    target.playerVelocityDecay = source.playerVelocityDecay;
    target.ballVelocityDecay = source.ballVelocityDecay;
    target.maxPlayerSpeed = source.maxPlayerSpeed;
    target.maxBallSpeed = source.maxBallSpeed;
    target.catchProbability = source.catchProbability;
    target.playerRandomNoise = source.playerRandomNoise;
    target.playerDirectionNoise = source.playerDirectionNoise;
    target.playerVelocityDirectionMix = source.playerVelocityDirectionMix;
    target.ballInsidePlayerVelocityDisplace = source.ballInsidePlayerVelocityDisplace;
    target.afterCatchDistance = source.afterCatchDistance;
}

const char R2Settings_doc[]= "Object containing the settings of the simulation.\n\n\
The method copy() returns a binary copy of this object.\n\
Converting an object to string and then evaluating that string with eval()\
will result in a dictionary containing all the fields of the object.\n\
E.g. my_dict=eval(str(my_settings))\n\n\
";

static PyTypeObject R2SettingsType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *
R2Settings_copy(R2SettingsObject *self, PyObject *Py_UNUSED(ignored))
{
    R2SettingsObject* target=PyObject_New(R2SettingsObject, &R2SettingsType);
    if(target==NULL)
    {
	    PyErr_SetString(R2Error, "unable to create settings object");
        return NULL;
    }
    target->simplified = self->simplified;
    target->ticksPerTime = self->ticksPerTime;
    target->pitchLength = self->pitchLength;
    target->pitchWidth = self->pitchWidth;
    target->goalWidth = self->goalWidth;
    target->centerRadius = self->centerRadius;
    target->poleRadius = self->poleRadius;   
    target->ballRadius = self->ballRadius;
    target->playerRadius = self->playerRadius;
    target->catchRadius = self->catchRadius;
    target->catchHoldingTicks = self->catchHoldingTicks;
    target->kickRadius = self->kickRadius;
    target->kickableDistance = self->kickableDistance;
    target->catchableDistance = self->catchableDistance;
    target->kickableAngle = self->kickableAngle;
    target->kickableDirectionAngle = self->kickableDirectionAngle;
    target->catchableAngle = self->catchableAngle;
    target->netLength = self->netLength;
    target->catchableAreaLength = self->catchableAreaLength;
    target->catchableAreaWidth = self->catchableAreaWidth;
    target->cornerMinDistance = self->cornerMinDistance;
    target->throwinMinDistance = self->throwinMinDistance;
    target->outPitchLimit = self->outPitchLimit;
    target->maxDashPower = self->maxDashPower;
    target->maxKickPower = self->maxKickPower;
    target->playerVelocityDecay = self->playerVelocityDecay;
    target->ballVelocityDecay = self->ballVelocityDecay;
    target->maxPlayerSpeed = self->maxPlayerSpeed;
    target->maxBallSpeed = self->maxBallSpeed;
    target->catchProbability = self->catchProbability;
    target->playerRandomNoise = self->playerRandomNoise;
    target->playerDirectionNoise = self->playerDirectionNoise;
    target->playerVelocityDirectionMix = self->playerVelocityDirectionMix;
    target->ballInsidePlayerVelocityDisplace = self->ballInsidePlayerVelocityDisplace;
    target->afterCatchDistance = self->afterCatchDistance;

    return (PyObject*)target;
}

static PyMethodDef R2Settings_methods[] = {
    {"copy", (PyCFunction) R2Settings_copy, METH_NOARGS,
     "Builds and returns a copy"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef R2Settings_members[] = {
    {(char*)"simplified", T_BOOL, offsetof(R2SettingsObject, simplified), 0, (char*)"boolean : set it to False if you want a more realistic ball control. Set it to True (default value) if you want a simplified simulation that eases the ball control to the agent and relaxes constraints on ball kicking (in this modality agents can kick the ball even behind their back towards any position, as if they can capture the ball from one side and release it from the other)"},  
    {(char*)"ticks_per_time", T_INT, offsetof(R2SettingsObject, ticksPerTime), 0, (char*)"integer : duration of each half time, expressed in ticks (a tick is a time step). The total duration of a match is 2*ticks_per_time (since there are 2 half times)"},
    {(char*)"pitch_length", T_DOUBLE, offsetof(R2SettingsObject, pitchLength), 0, (char*)"float : length of the pitch, that in the simulation extends along horizontal axes. pitch_length is the length of the playable zone, delimited by end lines. If during play the ball exit that zone there will be a goal-kick or a corner kick, but players can anyway move and dash outside that zone in a zone that is larger by the quantity out_pitch_limit (the actual coordinates are calculated in the object robosoc2d.pitch, in pitch.border_left and pitch.border_right"},
    {(char*)"pitch_width", T_DOUBLE, offsetof(R2SettingsObject, pitchWidth), 0, (char*)"float : width of the pitch, that in the simulation extends along vertical axes. pitch_width is the width of the playable zone, delimited by side lines. If during play the ball exit that zone there will be a throw-in, but players can anyway move and dash outside that zone in a zone that is larger by the quantity out_pitch_limit (the actual coordinates are calculated in the object robosoc2d.pitch, in pitch.border_up and pitch.border_down)"},
    {(char*)"goal_width", T_DOUBLE, offsetof(R2SettingsObject, goalWidth), 0, (char*)"float : width of goals, in vertical coordinates. It is the internal width, excluding poles. External width including poles instead is equal to (goal_width + (pole_radius*2)*2)"},
    {(char*)"center_radius", T_DOUBLE, offsetof(R2SettingsObject, centerRadius), 0, (char*)"float : radius of center circle of the pitch"},
    {(char*)"pole_radius", T_DOUBLE, offsetof(R2SettingsObject, poleRadius), 0, (char*)"float : radius of poles"},   
    {(char*)"ball_radius", T_DOUBLE, offsetof(R2SettingsObject, ballRadius), 0, (char*)"float : radius of the ball"},
    {(char*)"player_radius", T_DOUBLE, offsetof(R2SettingsObject, playerRadius), 0, (char*)"float : radius of the simulated circular player robot"},
    {(char*)"catch_radius", T_DOUBLE, offsetof(R2SettingsObject, catchRadius), 0, (char*)"float : maximum reach of goalkeepers, that contributes to determine catchable_distance"},
    {(char*)"catch_holding_ticks", T_INT, offsetof(R2SettingsObject, catchHoldingTicks), 0, (char*)"integer : duration in ticks of the period in which goalkeepers can hold the ball once catched"},
    {(char*)"kick_radius", T_DOUBLE, offsetof(R2SettingsObject, kickRadius), 0, (char*)"float : maximum length of a kick, that contributes to determine kickable_distance"},
    {(char*)"kickable_distance", T_DOUBLE, offsetof(R2SettingsObject, kickableDistance), 0, (char*)"float : distance to ball within which players can kick the ball. It equals (kick_radius+player_radius+ball_radius)"},
    {(char*)"catchable_distance", T_DOUBLE, offsetof(R2SettingsObject, catchableDistance), 0, (char*)"float : distance within which goalkeepers can catch the ball. It equals (catch_radius+player_radius+ball_radius)"},
    {(char*)"kickable_angle", T_DOUBLE, offsetof(R2SettingsObject, kickableAngle), 0, (char*)"float : maximum angle between player direction and player-ball direct line that permits the player to kick the ball (if the angle is greater the player will not be able to reach the ball for kicking)"},
    {(char*)"kickable_direction_angle", T_DOUBLE, offsetof(R2SettingsObject, kickableDirectionAngle), 0, (char*)"float : maximum angle between player direction and the direction of intendend kick to the ball. (The intended direction of a kick is usually expressed as in the first float value in the action tuple, after the integer value robosoc2d.ACTION_KICK). If the angle is greather than kickable_direction_angle, the player will not be able to kick the ball."},
    {(char*)"catchable_angle", T_DOUBLE, offsetof(R2SettingsObject, catchableAngle), 0, (char*)"float : maximum angle between goalkeeper direction and goalkeeper-ball direct line that permits the goalkeeper to catch the ball (if the angle is greater the goalkeeper will not be able to reach the ball to catch it)"},
    {(char*)"net_length", T_DOUBLE, offsetof(R2SettingsObject, netLength), 0, (char*)"float : the lenght, in horizontal units, of the net structure of goals."},
    {(char*)"catchable_area_length", T_DOUBLE, offsetof(R2SettingsObject, catchableAreaLength), 0, (char*)"float : the lenght, in horizontal units, of the catchable area, where goalkeepers can catch and hold the ball"},
    {(char*)"catchable_area_width", T_DOUBLE, offsetof(R2SettingsObject, catchableAreaWidth), 0, (char*)"float : the width, in horizontal units, of the catchable area, where goalkeepers can catch and hold the ball"},
    {(char*)"corner_min_distance", T_DOUBLE, offsetof(R2SettingsObject, cornerMinDistance), 0, (char*)"float : the minimum distance that opponents have to keep from ball during a corner kick. If opponents are closer, the simulator will automatically move them farther. "},
    {(char*)"throwin_min_distance", T_DOUBLE, offsetof(R2SettingsObject, throwinMinDistance), 0, (char*)"float : the minimum distance that opponents have to keep from ball during athrow-in. If opponents are closer, the simulator will automatically move them farther"},
    {(char*)"out_pitch_limit", T_DOUBLE, offsetof(R2SettingsObject, outPitchLimit), 0, (char*)"float : a distance external from pitch limits within which players can move and dash"},
    {(char*)"max_dash_power", T_DOUBLE, offsetof(R2SettingsObject, maxDashPower), 0, (char*)"float : maximum possible value for dash power (the dash power is usually expressed as second float value in the action tuple, after the integer value robosoc2d.ACTION_DASH and the first float value containing the dash direction)."},
    {(char*)"max_kick_power", T_DOUBLE, offsetof(R2SettingsObject, maxKickPower), 0, (char*)"float : maximum possible value for kick power (the kick power is usually expressed as second float value in the action tuple, after the integer value robosoc2d.ACTION_KICK and the first float value containing the kick direction)."},
    {(char*)"player_velocity_decay", T_DOUBLE, offsetof(R2SettingsObject, playerVelocityDecay), 0, (char*)"float :  decay in player velocity at each step. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details"},
    {(char*)"ball_velocity_decay", T_DOUBLE, offsetof(R2SettingsObject, ballVelocityDecay), 0, (char*)"float : decay in ball velocity at each step. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details"},
    {(char*)"max_player_speed", T_DOUBLE, offsetof(R2SettingsObject, maxPlayerSpeed), 0, (char*)"float : maximum speed reachable by players"},
    {(char*)"max_ball_speed", T_DOUBLE, offsetof(R2SettingsObject, maxBallSpeed), 0, (char*)"float : maximum speed reachable by ball"},
    {(char*)"catch_probability", T_DOUBLE, offsetof(R2SettingsObject,catchProbability), 0, (char*)"float : probability that goalkeepers succeed in catching the ball in case the ball is at appropriate distance and angle"},
    {(char*)"player_random_noise", T_DOUBLE, offsetof(R2SettingsObject,playerRandomNoise), 0, (char*)"float :  constant used to calculate and add some random noise to player dash and kicking power (and to ball movement). In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details"},
    {(char*)"player_direction_noise", T_DOUBLE, offsetof(R2SettingsObject, playerDirectionNoise), 0, (char*)"float : constant used to calculate and add some random noise to player dash and kicking direction. In common usage of the simulator you should not feel the need to either read or set this value. See the simulator source code for greater details"},
    {(char*)"player_velocity_direction_mix", T_DOUBLE, offsetof(R2SettingsObject, playerVelocityDirectionMix), 0, (char*)"float : constant used in the formula to calculate player inertia. In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details"},
    {(char*)"ball_inside_player_velocity_displace", T_DOUBLE, offsetof(R2SettingsObject, ballInsidePlayerVelocityDisplace), 0, (char*)"float : constant used in the formula to calculate player inertia. In common usage of the simulator you should not feel the need to either read or set this value.  See the simulator source code for greater details"},
    {(char*)"after_catch_distance", T_DOUBLE, offsetof(R2SettingsObject, afterCatchDistance), 0, (char*)"float : constant used to calculate the position of the ball when the goalkeeper that caught the ball releases the ball after the catch time has terminated. The ball will be approximately at a distance equal to (player_radius+ball_radius+after_catch_distance) from the position of the goalkeeper, along the goalkeeper direction vector"},
    {NULL}  /* Sentinel */
};

struct R2PitchObject {
    PyObject_HEAD
    double x1;
    double x2;
    double y1;
    double y2;
    double xGoal1;
    double xGoal2;
    double yGoal1;
    double yGoal2;
    double areaLx; 
    double areaRx;
    double areaUy;
    double areaDy;
    double goalKickLx;
    double goalKickRx;
    double goalKickUy;
    double goalKickDy;
    double pole1x;
    double pole1y;
    double pole2x;
    double pole2y; 
    double pole3x;
    double pole3y;
    double pole4x;
    double pole4y;
    double border_up;
    double border_down;
    double border_left;
    double border_right;
};

static PyObject *R2PitchObject_repr(R2PitchObject *obj){
    std::stringstream buffer;
    buffer << "{" 
        << "'x1': " << obj->x1 << ", "
        << "'x2': " << obj->x2 << ", "
        << "'y1': " << obj->y1 << ", "
        << "'y2': " << obj->y2 << ", "
        << "'x_goal1': " << obj->xGoal1 << ", "
        << "'x_goal2': " << obj->xGoal2 << ", "
        << "'y_goal1': " << obj->yGoal1 << ", "
        << "'y_goal2': " << obj->yGoal2 << ", "
        << "'area_lx': " << obj->areaLx << ", "
        << "'area_rx': " << obj->areaRx << ", "
        << "'area_uy': " << obj->areaUy << ", "
        << "'area_dy': " << obj->areaDy << ", "
        << "'goal_kick_lx': " << obj->goalKickLx << ", "
        << "'goal_kick_rx': " << obj->goalKickRx << ", "
        << "'goal_kick_uy': " << obj->goalKickUy << ", "
        << "'goal_kick_dy': " << obj->goalKickDy << ", "
        << "'pole1x': " << obj->pole1x << ", "
        << "'pole1y': " << obj->pole1y << ", "
        << "'pole2x': " << obj->pole2x << ", "
        << "'pole2y': " << obj->pole2y << ", "
        << "'pole3x': " << obj->pole3x << ", "
        << "'pole3y': " << obj->pole3y << ", "
        << "'pole4x': " << obj->pole4x << ", "
        << "'pole4y': " << obj->pole4y << ", "
        << "'border_up': " << obj->border_up << ", "
        << "'border_down': " << obj->border_down << ", "
        << "'border_left': " << obj->border_left << ", "
        << "'border_right': " << obj->border_right 
        << "}" ;
    return PyUnicode_FromString(buffer.str().c_str());
}

static void
R2PitchType_dealloc(R2PitchObject *obj)
{
    Py_TYPE(obj)->tp_free((PyObject *) obj);
}

void fillR2PitchObject(R2PitchObject& target, const R2Pitch& source){
    target.x1 = source.x1;
    target.x2 = source.x2;
    target.y1 = source.y1;
    target.y2 = source.y2;
    target.xGoal1 = source.xGoal1;
    target.xGoal2 = source.xGoal2;
    target.yGoal1 = source.yGoal1;
    target.yGoal2 = source.yGoal2;
    target.areaLx = source.areaLx; 
    target.areaRx = source.areaRx;
    target.areaUy = source.areaUy;
    target.areaDy = source.areaDy;
    target.goalKickLx = source.goalKickLx;
    target.goalKickRx = source.goalKickRx;
    target.goalKickUy = source.goalKickUy;
    target.goalKickDy = source.goalKickDy;
    target.pole1x = source.poles[0].x;
    target.pole1y = source.poles[0].y;
    target.pole2x = source.poles[1].x;
    target.pole2y = source.poles[1].y; 
    target.pole3x = source.poles[2].x;
    target.pole3y = source.poles[2].y;
    target.pole4x = source.poles[3].x;
    target.pole4y = source.poles[3].y;
    target.border_up = source.border_up;
    target.border_down = source.border_down;
    target.border_left = source.border_left;
    target.border_right = source.border_right;
}

const char R2Pitch_doc[] = "Object containing information about the game pitch.\n\n\
The method copy() returns a binary copy of this object.\n\
Converting an object to string and then evaluating that string with eval() will result in a dictionary containing all the fields of the object.\n\
E.g. my_dict=eval(str(my_pitch))\n\n";

static PyTypeObject R2PitchType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *
R2Pitch_copy(R2PitchObject *self, PyObject *Py_UNUSED(ignored))
{
    R2PitchObject* target=PyObject_New(R2PitchObject, &R2PitchType);
    if(target==NULL)
    {
	    PyErr_SetString(R2Error, "unable to create pitch object");
        return NULL;
    }

    target->x1 = self->x1;
    target->x2 = self->x2;
    target->y1 = self->y1;
    target->y2 = self->y2;
    target->xGoal1 = self->xGoal1;
    target->xGoal2 = self->xGoal2;
    target->yGoal1 = self->yGoal1;
    target->yGoal2 = self->yGoal2;
    target->areaLx = self->areaLx; 
    target->areaRx = self->areaRx;
    target->areaUy = self->areaUy;
    target->areaDy = self->areaDy;
    target->goalKickLx = self->goalKickLx;
    target->goalKickRx = self->goalKickRx;
    target->goalKickUy = self->goalKickUy;
    target->goalKickDy = self->goalKickDy;
    target->pole1x = self->pole1x;
    target->pole1y = self->pole1y;
    target->pole2x = self->pole2x;
    target->pole2y = self->pole2y;
    target->pole3x = self->pole3x;
    target->pole3y = self->pole3y;
    target->pole4x = self->pole4x;
    target->pole4y = self->pole4y;
    target->border_up = self->border_up;
    target->border_down = self->border_down;
    target->border_left = self->border_left;
    target->border_right = self->border_right;

    return (PyObject*)target;
}

static PyMethodDef R2Pitch_methods[] = {
    {"copy", (PyCFunction) R2Pitch_copy, METH_NOARGS,
     "Builds and returns a copy"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef R2Pitch_members[] = {
    {(char*)"x1", T_DOUBLE, offsetof(R2PitchObject, x1), 0, (char*)"float : horizontal coordinate of right pitch boundary (usually a positive number)"},
    {(char*)"x2", T_DOUBLE, offsetof(R2PitchObject, x2), 0, (char*)"float : horizontal coordinate of left pitch boundary (usually a negative number = -x1)"},
    {(char*)"y1", T_DOUBLE, offsetof(R2PitchObject, y1), 0, (char*)"float : vertical coordinate of upper pitch boundary (usually a postiive number)"},
    {(char*)"y2", T_DOUBLE, offsetof(R2PitchObject, y2), 0, (char*)"float : vertical coordinate of lower pitch boundary (usually a negative number = -y1)"},    
    {(char*)"x_goal1", T_DOUBLE, offsetof(R2PitchObject, xGoal1), 0, (char*)"float : horizontal coordinate of the right end of the right goal considering the net. The right goal begins at coordinate x1 (at the right pitch boundary, where if the ball surpass the vertical line you score a goal) and it ends at coordinate x_goal1 "},
    {(char*)"x_goal2", T_DOUBLE, offsetof(R2PitchObject, xGoal2), 0, (char*)"float : horizontal coordinate of the left end of the left goal considering the net. The lect goal begins at coordinate x2 (at the left pitch boundary, where if the ball surpass the vertical line you score a goal) and it ends at coordinate x_goal2"},
    {(char*)"y_goal1", T_DOUBLE, offsetof(R2PitchObject, yGoal1), 0, (char*)"float : vertical coordinate of the upper end of the goals, where you have upper poles"},
    {(char*)"y_goal2", T_DOUBLE, offsetof(R2PitchObject, yGoal2), 0, (char*)"float : vertical coordinate of the lower end of the goals, where you have lower poles"},
    {(char*)"area_lx", T_DOUBLE, offsetof(R2PitchObject, areaLx), 0, (char*)"float : right horizontal boundary of the left area (the left area has left horizontal boundary in x2)"},
    {(char*)"area_rx", T_DOUBLE, offsetof(R2PitchObject, areaRx), 0, (char*)"float : left horizontal boundary of the right area (the right area has right horizontal boundary in x1)"},
    {(char*)"area_uy", T_DOUBLE, offsetof(R2PitchObject, areaUy), 0, (char*)"float : upper vertical boundary of areas"},
    {(char*)"area_dy", T_DOUBLE, offsetof(R2PitchObject, areaDy), 0, (char*)"float : lower (down) vertical boundary of areas"},
    {(char*)"goal_kick_lx", T_DOUBLE, offsetof(R2PitchObject, goalKickLx), 0, (char*)"float : horizontal coordinate of left goal kicks position"},
    {(char*)"goal_kick_rx", T_DOUBLE, offsetof(R2PitchObject, goalKickRx), 0, (char*)"float : horizontal coordinate of right goal kicks position"},
    {(char*)"goal_kick_uy", T_DOUBLE, offsetof(R2PitchObject, goalKickUy), 0, (char*)"float : vertical coordinate of upper goal kicks position"},
    {(char*)"goal_kick_dy", T_DOUBLE, offsetof(R2PitchObject, goalKickDy), 0, (char*)"float : vertical coordinate of lower (down) goal kicks position"},
    {(char*)"pole1x", T_DOUBLE, offsetof(R2PitchObject, pole1x), 0, (char*)"float : horizontal coordinate of left upper pole"},
    {(char*)"pole1y", T_DOUBLE, offsetof(R2PitchObject, pole1y), 0, (char*)"float : vertical coordinate of left upper pole"},
    {(char*)"pole2x", T_DOUBLE, offsetof(R2PitchObject, pole2x), 0, (char*)"float : horizontal coordinate of left lower pole"},
    {(char*)"pole2y", T_DOUBLE, offsetof(R2PitchObject, pole2y), 0, (char*)"float : vertical coordinate of the left lower pole"}, 
    {(char*)"pole3x", T_DOUBLE, offsetof(R2PitchObject, pole3x), 0, (char*)"float : horizontal coordinate of right upper pole"},
    {(char*)"pole3y", T_DOUBLE, offsetof(R2PitchObject, pole3y), 0, (char*)"float : vertical coordinate of right upper pole"},
    {(char*)"pole4x", T_DOUBLE, offsetof(R2PitchObject, pole4x), 0, (char*)"float : horizontal coordinate of right lower pole"},
    {(char*)"pole4y", T_DOUBLE, offsetof(R2PitchObject, pole4y), 0, (char*)"float : vertical coordinate of right lower pole"},
    {(char*)"border_up", T_DOUBLE, offsetof(R2PitchObject, border_up), 0, (char*)"float : vertical coordinate of the extreme upper end of the playfield, the more external upper coordinate where a player can be (it's outside the pitch"},
    {(char*)"border_down", T_DOUBLE, offsetof(R2PitchObject, border_down), 0, (char*)"float : vertical coordinate of the extreme lower end of the playfield, the more external lower coordinate where a player can be (it's outside the pitch"},
    {(char*)"border_left", T_DOUBLE, offsetof(R2PitchObject, border_left), 0, (char*)"float : horizontal coordinate of the extreme left end of the playfield, the more external left coordinate where a player can be (it's outside the pitch)"},
    {(char*)"border_right", T_DOUBLE, offsetof(R2PitchObject, border_right), 0, (char*)"float : horizontal coordinate of the extreme right end of the playfield, the more external right coordinate where a player can be (it's outside the pitch)"},
    {NULL}  /* Sentinel */
};

struct R2EnvironmentObject {
    PyObject_HEAD
    int tick;    
    int score1;
    int score2;
    int state;
    double ballX;
    double ballY;
    double ballVelocityX;
    double ballVelocityY;
    bool lastTouchedTeam2;  
    double startingTeamMaxRange;   
    int ballCatched;        
    bool ballCatchedTeam2;
    bool halftimePassed;
    int nPlayers1;
    int nPlayers2;
};

static PyObject *R2EnvironmentObject_repr(R2EnvironmentObject *obj){
    std::stringstream buffer;
    buffer << "{" 
        << "'tick': " << obj->tick << ", "
        << "'score1': " << obj->score1 << ", "
        << "'score2': " << obj->score2 << ", "
        << "'state': " << obj->state << ", "
        << "'ball_x': " << obj->ballX << ", "
        << "'ball_y': " << obj->ballY << ", "
        << "'ball_velocity_x': " << obj->ballVelocityX << ", "
        << "'ball_velocity_y': " << obj->ballVelocityY << ", "
        << "'last_touched_team2': " << (obj->lastTouchedTeam2 ? "True" : "False") << ", "
        << "'starting_team_max_range': " << obj->startingTeamMaxRange << ", "
        << "'ball_catched': " << obj->ballCatched << ", "
        << "'ball_catched_team2': " << (obj-> ballCatchedTeam2 ? "True" : "False") << ", "
        << "'halftime_passed': " << (obj-> halftimePassed ? "True" : "False") << ", "
        << "'n_players1': " << obj->nPlayers1 << ", "
        << "'n_players2': " << obj->nPlayers2 
        << "}" ;
    return PyUnicode_FromString(buffer.str().c_str());
}

static void
R2EnvironmentType_dealloc(R2EnvironmentObject *obj)
{
    Py_TYPE(obj)->tp_free((PyObject *) obj);
}


void fillR2EnvironmentObject(R2EnvironmentObject& target, const R2Environment& source){
    target.tick = source.tick;    
    target.score1 = source.score1;
    target.score2 = source.score2;
    target.state = static_cast<int>(source.state);
    target.ballX = source.ball.pos.x;
    target.ballY = source.ball.pos.y;
    target.ballVelocityX = source.ball.velocity.x;
    target.ballVelocityY = source.ball.velocity.y;
    target.lastTouchedTeam2 = source.lastTouchedTeam2;  
    target.startingTeamMaxRange = source.startingTeamMaxRange;   
    target.ballCatched = source.ballCatched;        
    target.ballCatchedTeam2 = source.ballCatchedTeam2;  
    target.halftimePassed = source.halftimePassed;  
    target.nPlayers1 = source.teams[0].size();
    target.nPlayers2 = source.teams[1].size();
}

const char R2Environment_doc[] = "Object containing the state of the simulation environment.\n\n\
The method copy() returns a binary copy of this object.\n\
Converting an object to string and then evaluating that string with eval() will result in a dictionary containing all the fields of the object.\n\
E.g. my_dict=eval(str(my_environment))\n\n";

static PyTypeObject R2EnvironmentType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *
R2Environment_copy(R2EnvironmentObject *self, PyObject *Py_UNUSED(ignored))
{
    R2EnvironmentObject* target=PyObject_New(R2EnvironmentObject, &R2EnvironmentType);
    if(target==NULL)
    {
	    PyErr_SetString(R2Error, "unable to create environment object");
        return NULL;
    }

    target->tick = self->tick;    
    target->score1 = self->score1;
    target->score2 = self->score2;
    target->state = self->state;
    target->ballX = self->ballX;
    target->ballY = self->ballY;
    target->ballVelocityX = self->ballVelocityX;
    target->ballVelocityY = self->ballVelocityY;
    target->lastTouchedTeam2 = self->lastTouchedTeam2;  
    target->startingTeamMaxRange = self->startingTeamMaxRange;   
    target->ballCatched = self->ballCatched;        
    target->ballCatchedTeam2 = self->ballCatchedTeam2;  
    target->halftimePassed = self->halftimePassed;  
    target->nPlayers1 = self->nPlayers1;
    target->nPlayers2 = self->nPlayers2;

    return (PyObject*)target;
}

static PyMethodDef R2Environment_methods[] = {
    {"copy", (PyCFunction) R2Environment_copy, METH_NOARGS,
     "Builds and returns a copy"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef R2Environment_members[] = {
    {(char*)"tick", T_INT, offsetof(R2EnvironmentObject, tick), 0, (char*)"integer : the current tick (a tick is a timestep of the game)"},
    {(char*)"score1", T_INT, offsetof(R2EnvironmentObject, score1), 0, (char*)"integer : the score by first team"},
    {(char*)"score2", T_INT, offsetof(R2EnvironmentObject, score2), 0, (char*)"integer : the score by second team"},
    {(char*)"state", T_INT, offsetof(R2EnvironmentObject, state), 0, (char*)"integer : an integer representing the state of the game, such as robosoc2d.STATE_PLAY . Possible values, all inside the robosoc2d module, are: STATE_INACTIVE, STATE_READY (unused), STATE_KICKOFF1, STATE_KICKOFF2, STATE_PLAY, STATE_STOPPED (unused), STATE_GOALKICK1UP, STATE_GOALKICK1DOWN, STATE_GOALKICK2UP, STATE_GOALKICK2DOWN, STATE_CORNER1UP, STATE_CORNER1DOWN, STATE_CORNER2UP, STATE_CORNER2DOWN, STATE_THROWIN1, STATE_THROWIN2, STATE_PAUSED (unused), STATE_HALFTIME, STATE_GOAL1, STATE_GOAL2, STATE_ENDED"},
    {(char*)"ball_x", T_DOUBLE, offsetof(R2EnvironmentObject, ballX), 0, (char*)"float : horizontal coordinate of ball position"},   
    {(char*)"ball_y", T_DOUBLE, offsetof(R2EnvironmentObject, ballY), 0, (char*)"float : vertical coordinate of ball position"},   
    {(char*)"ball_velocity_x", T_DOUBLE, offsetof(R2EnvironmentObject, ballVelocityX), 0, (char*)"float : horizontal component of ball's velocity vector"},   
    {(char*)"ball_velocity_y", T_DOUBLE, offsetof(R2EnvironmentObject, ballVelocityY), 0, (char*)"float : vertical component of ball's velocity vector"},   
    {(char*)"last_touched_team2", T_BOOL, offsetof(R2EnvironmentObject, lastTouchedTeam2), 0, (char*)"boolean : set to True if last player to touch or kick the ball was belonging to second team, or set to False if belonging to first team"},  
    {(char*)"starting_team_max_range", T_DOUBLE, offsetof(R2EnvironmentObject, startingTeamMaxRange), 0, (char*)"float : maximum range of movement for players of the non-kicking team in case of throw-in, corner kick, or goal-kick"},     
    {(char*)"ball_catched", T_INT, offsetof(R2EnvironmentObject, ballCatched), 0, (char*)"integer : number of ticks still available to goalkeeper to hold the ball"},        
    {(char*)"ball_catched_team2", T_BOOL, offsetof(R2EnvironmentObject, ballCatchedTeam2), 0, (char*)"boolean : in case a goalkeeper has catched the ball this attribute is set to True if it's the second team's goalkeeper, or set to False if it's the first team's goalkeeper"},  
    {(char*)"halftime_passed", T_BOOL, offsetof(R2EnvironmentObject, halftimePassed), 0, (char*)"boolean : this is set to True after the second half of the match has begun, otherwise it is set to False"},  
    {(char*)"n_players1", T_INT, offsetof(R2EnvironmentObject, nPlayers1), 0, (char*)"integer : number of players of first team"},
    {(char*)"n_players2", T_INT, offsetof(R2EnvironmentObject, nPlayers2), 0, (char*)"integer : number of players of second team"},
    {NULL}  /* Sentinel */
};

struct R2PlayerInfoObject {
    PyObject_HEAD
    double x;
    double y;
    double velocityX;
    double velocityY;
    double direction; 
    bool acted; 
};

static PyObject *R2PlayerInfoObject_repr(R2PlayerInfoObject *obj){
    std::stringstream buffer;
    buffer << "{" 
        << "'x': " << obj->x << ", "
        << "'y': " << obj->y << ", "
        << "'velocity_x': " << obj->velocityX << ", "
        << "'velocity_y': " << obj->velocityY << ", "
        << "'direction': " << obj->direction << ", "
        << "'acted': " << (obj->acted ? "True" : "False")
        << "}" ;
    return PyUnicode_FromString(buffer.str().c_str());
}

static void
R2PlayerInfoType_dealloc(R2PlayerInfoObject *obj)
{
    Py_TYPE(obj)->tp_free((PyObject *) obj);
}

void fillR2PlayerInfoObject(R2PlayerInfoObject& target, const R2PlayerInfo& source){
    target.x = source.pos.x;
    target.y = source.pos.y;
    target.velocityX = source.velocity.x;
    target.velocityY = source.velocity.y;
    target.direction = source.direction; 
    target.acted = source.acted; 
}

void setR2PlayerInfo(R2PlayerInfo& target, const R2PlayerInfoObject& source){
    target.pos.x = source.x;
    target.pos.y = source.y;
    target.velocity.x = source.velocityX;
    target.velocity.y = source.velocityY;
    target.direction = source.direction; 
    target.acted = source.acted; 
}

const char R2PlayerInfo_doc[] = "Object representing information about a player.\n\n\
The method copy() returns a binary copy of this object.\n\
Converting an object to string and then evaluating that string with eval() will result in a dictionary containing all the fields of the object.\n\
E.g. my_dict=eval(str(my_player_info))\n\n";

static PyTypeObject R2PlayerInfoType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *
R2PlayerInfo_copy(R2PlayerInfoObject *self, PyObject *Py_UNUSED(ignored))
{
    R2PlayerInfoObject* target=PyObject_New(R2PlayerInfoObject, &R2PlayerInfoType);
    if(target==NULL)
    {
	    PyErr_SetString(R2Error, "unable to create environment object");
        return NULL;
    }

    target->x = self->x;
    target->y = self->y;
    target->velocityX = self->velocityX;
    target->velocityY = self->velocityY;
    target->direction = self->direction; 
    target->acted = self->acted; 

    return (PyObject*)target;
}

static PyMethodDef R2PlayerInfo_methods[] = {
    {"copy", (PyCFunction) R2PlayerInfo_copy, METH_NOARGS,
     "Builds and returns a copy"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef R2PlayerInfo_members[] = {
    {(char*)"x", T_DOUBLE, offsetof(R2PlayerInfoObject, x), 0, (char*)"float : horizontal coordinate of player position"},
    {(char*)"y", T_DOUBLE, offsetof(R2PlayerInfoObject, y), 0, (char*)"float : vertical coordinate of player position"},
    {(char*)"velocity_x", T_DOUBLE, offsetof(R2PlayerInfoObject, velocityX), 0, (char*)"float : horizontal component of player's velocity vector"},
    {(char*)"velocity_y", T_DOUBLE, offsetof(R2PlayerInfoObject, velocityY), 0, (char*)"float : vertical component of player's velocity vector"},
    {(char*)"direction", T_DOUBLE, offsetof(R2PlayerInfoObject, direction), 0, (char*)"float : direction of the player, expressed in radians"}, 
    {(char*)"acted", T_BOOL, offsetof(R2PlayerInfoObject, acted), 0, (char*)"float : set to True if player has alraedy acted in current Tick"}, 
    {NULL}  /* Sentinel */
};

// returns a tuple containing environment, pitch, settings, team1 info, team2 info
// the reference is stolen by caller, that will own it
static PyObject* pythonizeGameState(const R2GameState& gameState) {
    R2EnvironmentObject* env=PyObject_New(R2EnvironmentObject, &R2EnvironmentType);
    if(env==NULL)
        return NULL;
    fillR2EnvironmentObject(*env, gameState.env);

    R2PitchObject* pitch=PyObject_New(R2PitchObject, &R2PitchType);
    if(pitch==NULL)
    {
        Py_DECREF(env);
        return NULL;
    }
    fillR2PitchObject(*pitch, gameState.pitch);

    R2SettingsObject* sett=PyObject_New(R2SettingsObject, &R2SettingsType);
    if(sett==NULL){
        Py_DECREF(env);
        Py_DECREF(pitch);
        return NULL;
    }
    fillR2SettingsObject(*sett, gameState.sett);

    PyObject *players1 = PyTuple_New(gameState.env.teams[0].size());
    if(players1==NULL){
        Py_DECREF(env);
        Py_DECREF(pitch);
        Py_DECREF(sett);
        return NULL;
    }

    for(unsigned int i=0; i<gameState.env.teams[0].size(); i++){
        R2PlayerInfoObject* playerInfo=PyObject_New(R2PlayerInfoObject, &R2PlayerInfoType);
        if(sett==NULL){
            Py_DECREF(env);
            Py_DECREF(pitch);
            Py_DECREF(sett);
            Py_DECREF(players1);
            return NULL;
        }
        fillR2PlayerInfoObject(*playerInfo, gameState.env.teams[0][i]);
        PyTuple_SetItem(players1, i, (PyObject*)playerInfo);
    }

    PyObject *players2 = PyTuple_New(gameState.env.teams[1].size());
    if(players1==NULL){
        Py_DECREF(env);
        Py_DECREF(pitch);
        Py_DECREF(sett);
        Py_DECREF(players1);
        return NULL;
    }

    for(unsigned int i=0; i<gameState.env.teams[1].size(); i++){
        R2PlayerInfoObject* playerInfo=PyObject_New(R2PlayerInfoObject, &R2PlayerInfoType);
        if(sett==NULL){
            Py_DECREF(env);
            Py_DECREF(pitch);
            Py_DECREF(sett);
            Py_DECREF(players1);
            Py_DECREF(players2);
            return NULL;
        }
        fillR2PlayerInfoObject(*playerInfo, gameState.env.teams[1][i]);
        PyTuple_SetItem(players2, i, (PyObject*)playerInfo);
    }

    PyObject *pyGameState = PyTuple_New(5);
    if(pyGameState==NULL){
        Py_DECREF(env);
        Py_DECREF(pitch);
        Py_DECREF(sett);
        Py_DECREF(players1);
        Py_DECREF(players2);
        return NULL;
    }
    PyTuple_SetItem(pyGameState, 0, (PyObject*)env);
    PyTuple_SetItem(pyGameState, 1, (PyObject*)pitch);
    PyTuple_SetItem(pyGameState, 2, (PyObject*)sett);
    PyTuple_SetItem(pyGameState, 3, (PyObject*)players1);
    PyTuple_SetItem(pyGameState, 4, (PyObject*)players2);
    return pyGameState;
}

static PyObject *stepMethodName;
class PythonPlayer : public R2Player {
private:
    PyObject *pythonPlayerObject;
public:
    PythonPlayer(PyObject *player);
    ~PythonPlayer(); 

    virtual R2Action step(const R2GameState gameState) override;
};

R2Action PythonPlayer::step(const R2GameState gameState) {
    if(pythonPlayerObject!=NULL){
        PyObject *args =pythonizeGameState(gameState);
        if(args==NULL){
            return R2Action();
        }

        // python player step() method
        PyObject *result = PyObject_CallMethodObjArgs(pythonPlayerObject, stepMethodName, PyTuple_GetItem(args,0), PyTuple_GetItem(args,1),
                                                        PyTuple_GetItem(args,2), PyTuple_GetItem(args,3), PyTuple_GetItem(args,4), NULL);  // NOTE: final NULL is necessary to signal NULL-termination of arguments!
        Py_DECREF(args);
                
        // NOW ELABORATE result
        if(result == NULL){
            return R2Action();
        }

        if(!PySequence_Check(result)){
            Py_DECREF(result);  //check
            return R2Action();
        }

        int len=(int)PySequence_Length(result);
        if(len != 4){
            Py_DECREF(result);  //check
            return R2Action();
        }

        R2Action playerAction;

        PyObject *po = PySequence_GetItem(result, 0);
        if (po == NULL){ 
            Py_DECREF(result);  //check
            return R2Action();
        }
        if(!PyLong_Check(po)){
            Py_DECREF(po);
            Py_DECREF(result);  //check
            return R2Action();
        }
        playerAction.action=static_cast<R2ActionType>((int)PyLong_AsLong(po));
        Py_DECREF(po);

        for(int i=0; i<3; i++){
            po = PySequence_GetItem(result, i+1);
            if (po == NULL){ 
                return R2Action();
            }
            if(!PyFloat_Check(po)){
                Py_DECREF(po);
                return R2Action();
            }
            playerAction.data[i]=PyFloat_AsDouble(po);
            Py_DECREF(po);
        }
        Py_DECREF(result);  //check

        return playerAction;    // only point in which the legit action from the python player is built and used
    }
    return R2Action();
}

PythonPlayer::PythonPlayer(PyObject *player){
    pythonPlayerObject=player;
    Py_XINCREF(pythonPlayerObject);
}

PythonPlayer::~PythonPlayer(){
    if(isPythonActive)
        Py_XDECREF(pythonPlayerObject);
} 

static int serialSimulations=1;

static auto simulations = std::unordered_map<int, std::shared_ptr<R2Simulator>> {};

static PyObject *robosoc2d_getVersion(PyObject *self, PyObject *args){
    return PyUnicode_FromString(GetR2SVersion());
}

static PyObject *robosoc2d_getDefaultSettings(PyObject *self, PyObject *args){
    R2SettingsObject* sett=PyObject_New(R2SettingsObject, &R2SettingsType);
    if(sett==NULL){
        PyErr_SetString(R2Error, "unable to create settings object");
        return NULL;
    }
    fillR2SettingsObject(*sett, R2EnvSettings());
    return (PyObject*)sett;
}

static PyObject *robosoc2d_getSeedByCurrentTime(PyObject *self, PyObject *args){
    int randomSeed=(int)createChronoRandomSeed();
    return PyLong_FromLong(randomSeed);
}

// return the handle of the simulator, 0 if failed
static int createSimulator(std::vector<std::shared_ptr<R2Player>> team1, std::vector<std::shared_ptr<R2Player>> team2,
     std::string team1name, std::string team2name,
     unsigned int random_seed= createChronoRandomSeed(),
      R2EnvSettings settings= R2EnvSettings()){

    int key=serialSimulations;
    serialSimulations++;

    auto[iterator, success] = simulations.emplace(
                        std::make_pair(key, std::make_shared<R2Simulator>(team1, team2, team1name, team2name, random_seed, settings) )
    );
    
    if(success)
        return key;
    
    return 0;
}

// Sequence of robosoc2d.player: provided team1 with user's logic
// Sequence of robosoc2d.player: provided team2 with user's logic
// integer (optional): random seed to be used by the simulation random number generators
// reference to a robosoc2d.settings object (optional): settings to be used to build the simulation
static PyObject *robosoc2d_buildSimulator(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *keywords[] = {(char *)"team1", (char *)"team2", (char *)"team1name", (char *)"team2name", (char *)"random_seed", (char *)"game_settings", NULL};
    int handle=0;
    PyObject *pTeamSequence[2]{NULL,NULL};
    const char* teamNames[2]{NULL,NULL};
    int randomSeed=(int)createChronoRandomSeed();
    PyObject *pObj=NULL;
    R2SettingsObject *pSettings;
    R2EnvSettings cSettings;

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|ssiO", keywords, &pTeamSequence[0], &pTeamSequence[1], &teamNames[0], &teamNames[1], &randomSeed, &pObj)){
        PyErr_SetString(PyExc_TypeError, "wrong parameters");
        return NULL;
    }

    std::vector<std::shared_ptr<R2Player>> team[2];

    for(int t=0; t<=1; t++){
        if(!PySequence_Check(pTeamSequence[t])){
            PyErr_SetString(PyExc_TypeError, "first and second arguments must be a sequence of players");
            return NULL;
        }

        int len=(int)PySequence_Length(pTeamSequence[t]);

        for (int i = 0; i < len; i++) { 
            PyObject *player = PySequence_GetItem(pTeamSequence[t], i);
            if (player == NULL){ // some failure
                PyErr_SetString(PyExc_TypeError, "unable to get player object from sequence");
                return NULL;
            }

            if(! PyObject_HasAttr(player, stepMethodName)){
                PyErr_SetString(PyExc_TypeError, "players must implement method function 'step' ");
                Py_DECREF(player);
                return NULL;
            }
            
            team[t].push_back(std::static_pointer_cast<R2Player>(std::make_shared<PythonPlayer>(player)));
            Py_DECREF(player);
        }
    }

    std::string name1((teamNames[0]!=NULL)? teamNames[0] : defaultPythonTeam1Name);
    std::string name2((teamNames[1]!=NULL)? teamNames[1] : defaultPythonTeam2Name);

    if(pObj != NULL){
        if(pObj->ob_type != &R2SettingsType){
            PyErr_SetString(PyExc_TypeError, "wrong type for settings parameter");
            return NULL;
        }
        pSettings=(R2SettingsObject*)pObj;
        fillR2Settings(cSettings, *pSettings);
        handle= createSimulator(team[0], team[1], name1, name2, (unsigned int)randomSeed, cSettings);
    }
    else{  
        handle= createSimulator(team[0], team[1], name1, name2, (unsigned int)randomSeed);
    }

    if (handle!=0)
        return PyLong_FromLong(handle);

    PyErr_SetString(R2Error, "impossible to create simulator");
    return NULL;
}

// Sequence of robosoc2d.player: provided team1 with user's logic
// integer: number of simpleplayer to add as first team1 players (the first SimplePlayer will play as goalkeeper)
// Sequence of robosoc2d.player: provided team2 with user's logic
// integer: number of simpleplayer to add as first team2 players (the first SimplePlayer will play as goalkeeper)
// integer (optional): random seed to be used by the simulation random number generators
// reference to a robosoc2d.settings object (optional): settings to be used to build the simulation
static PyObject *robosoc2d_buildSimplePlayerSimulator(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *keywords[] = {(char *)"team1", (char *)"how_manysimpleplayers_team1", (char *)"team2", (char *)"how_manysimpleplayers_team1", (char *)"team1name", (char *)"team2name", (char *)"random_seed", (char *)"game_settings", NULL};
    int handle=0;
    PyObject *pTeamSequence[2]{NULL,NULL};
    int createSimplePlayers[2]{0,0};
    const char* teamNames[2]{NULL,NULL};
    int randomSeed=(int)createChronoRandomSeed();
    PyObject *pObj=NULL;
    R2SettingsObject *pSettings;
    R2EnvSettings cSettings;

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OiOi|ssiO", keywords, &pTeamSequence[0], &createSimplePlayers[0], &pTeamSequence[1], &createSimplePlayers[1],  &teamNames[0], &teamNames[1], &randomSeed, &pObj)){
        PyErr_SetString(PyExc_TypeError, "wrong parameters");
        return NULL;
    }

    std::vector<std::shared_ptr<R2Player>> team[2];

    for(int t=0; t<=1; t++){
        for(int i=0; i<createSimplePlayers[t]; i++){
            team[t].push_back(std::static_pointer_cast<R2Player>(std::make_shared<SimplePlayer>(i, t)));
        }
    
        if(!PySequence_Check(pTeamSequence[t])){
            PyErr_SetString(PyExc_TypeError, "second argument must be a sequence of players");
            return NULL;
        }

        int len=(int)PySequence_Length(pTeamSequence[t]);

        for (int i = 0; i < len; i++) { 
            PyObject *player = PySequence_GetItem(pTeamSequence[t], i);
            if (player == NULL){ // some failure
                PyErr_SetString(PyExc_TypeError, "unable to get player object from sequence");
                return NULL;
            }

            if(! PyObject_HasAttr(player, stepMethodName)){
                PyErr_SetString(PyExc_TypeError, "players must implement method function 'step' ");
                Py_DECREF(player);
                return NULL;
            }
            
            team[t].push_back(std::static_pointer_cast<R2Player>(std::make_shared<PythonPlayer>(player)));
            Py_DECREF(player);
        }
    }

    std::string name1((teamNames[0]!=NULL)? teamNames[0] : defaultPythonTeam1Name);
    std::string name2((teamNames[1]!=NULL)? teamNames[1] : defaultPythonTeam2Name);

    if(pObj != NULL){
        if(pObj->ob_type != &R2SettingsType){
            PyErr_SetString(PyExc_TypeError, "wrong type for settings parameter");
            return NULL;
        }
        pSettings=(R2SettingsObject*)pObj;
        fillR2Settings(cSettings, *pSettings);
        handle= createSimulator(team[0], team[1], name1, name2, (unsigned int)randomSeed, cSettings);
    }
    else{  
        handle= createSimulator(team[0], team[1], name1, name2, (unsigned int)randomSeed);
    }

    if (handle!=0)
        return PyLong_FromLong(handle);

    PyErr_SetString(R2Error, "impossible to create simulator");
    return NULL;
}


static PyObject *robosoc2d_simulatorStepIfPlaying(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return PyBool_FromLong(long(simulations[handle]->stepIfPlaying()));
}

static PyObject *robosoc2d_simulatorDelete(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return PyBool_FromLong(simulations.erase(handle));
}

static PyObject *robosoc2d_simulatorPlayGame(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    while(simulations[handle]->stepIfPlaying()){
        //R2Environment env=simulations[handle]->getGameState().env;

        DEBUG_OUT(simulations[handle]->getStateString().c_str() );
        DEBUG_OUT("\n");
    }
    DEBUG_OUT(simulations[handle]->getStateString().c_str() ); // show also what happened in last tick
    DEBUG_OUT("\n");

    return PyBool_FromLong(1L);
}

static PyObject *robosoc2d_simulatorIsValid(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        return PyBool_FromLong(0L);
    }

    return PyBool_FromLong(1L);
}

static PyObject *robosoc2d_simulatorGetStateString(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return  PyUnicode_FromString(simulations[handle]->getStateString().c_str());
}

static PyObject *robosoc2d_simulatorGetGameState(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return  pythonizeGameState(simulations[handle]->getGameState());
}

static PyObject *robosoc2d_simulatorGetRandomSeed(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return  PyLong_FromLong(simulations[handle]->getRandomSeed());
}

static PyObject *robosoc2d_simulatorGetTeamNames(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", NULL};
    int handle;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", keywords, &handle)){
        PyErr_SetString(PyExc_TypeError, "parameter must be an integer");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    auto names=simulations[handle]->getTeamNames();
    PyObject *name1= PyUnicode_FromString(names[0].c_str());
    if(name1==NULL){
        return NULL;
    }
    PyObject *name2= PyUnicode_FromString(names[1].c_str());
    if(name2==NULL){
        Py_DECREF(name1);
        return NULL;
    }

    PyObject *pyNames = PyTuple_New(2);
    if(pyNames==NULL){
        Py_DECREF(name1);
        Py_DECREF(name2);
        return NULL;
    }
    PyTuple_SetItem(pyNames, 0, name1);
    PyTuple_SetItem(pyNames, 1, name2);
    
    return  pyNames;
}

static PyObject *robosoc2d_simulatorSaveStateHistory(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", (char *)"filename", NULL};
    int handle;
    const char* filename;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "is", keywords, &handle, &filename)){
        PyErr_SetString(PyExc_TypeError, "wrong parameters");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return  PyBool_FromLong((long)simulations[handle]->saveStatesHistory(string(filename)));
}

static PyObject *robosoc2d_simulatorSaveActionsHistory(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"handle", (char *)"filename", NULL};
    int handle;
    const char* filename;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "is", keywords, &handle, &filename)){
        PyErr_SetString(PyExc_TypeError, "wrong arguments");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    return  PyBool_FromLong((long)simulations[handle]->saveActionsHistory(string(filename)));
}

static PyObject *robosoc2d_simulatorDeleteAll(PyObject *self, PyObject *args){
    simulations.clear();
    Py_INCREF(Py_None);
    return Py_None;
}

// Sequence of robosoc2d.player: provided team1 with user's logic
// Sequence of robosoc2d.player: provided team2 with user's logic
// integer (optional): random seed to be used by the simulation random number generators
// reference to a robosoc2d.settings object (optional): settings to be used to build the simulation
static PyObject *robosoc2d_set_environment(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *keywords[] = {(char *)"handle", (char *)"tick", (char *)"score1",  (char *)"score2", (char *)"state",  (char *)"ball_x",  (char *)"ball_y",  (char *)"ball_velocity_x",  (char *)"ball_velocity_y", (char *)"team1", (char *)"team2",  (char *)"last_touched_team2",  (char *)"ball_catched",  (char *)"ball_catched_team2", NULL};
    int handle=0;
    int tick=0;
    int score1=0;
    int score2=0;
    int state=0;
    double ballX=0;
    double ballY=0;
    double ballVelocityX=0;
    double ballVelocityY=0;
    PyObject *pTeamSequence[2]{NULL,NULL};
    int lastTouchedTeam2= 0;
    int ballCatched=0;
    int ballCatchedTeam2=0;

    PyObject *pObj=NULL;

    R2SettingsObject *pSettings;
    R2EnvSettings cSettings;

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "iiiiiddddOO|pip", keywords, &handle, &tick, &score1, &score2, &state, &ballX, &ballY, &ballVelocityX, &ballVelocityY, &pTeamSequence[0], &pTeamSequence[1], &lastTouchedTeam2, &ballCatched, &ballCatchedTeam2)){
        PyErr_SetString(PyExc_TypeError, "wrong parameters");
        return NULL;
    }

    if(simulations.count(handle) == 0){
        PyErr_SetString(R2Error, "simulation handle not existing");
        return NULL;
    }

    R2State rstate= static_cast<R2State>(state);
    R2ObjectInfo ball(ballX, ballY, ballVelocityX, ballVelocityY);

    std::vector<R2PlayerInfo> team[2];

    for(int t=0; t<=1; t++){
        if(!PySequence_Check(pTeamSequence[t])){
            PyErr_SetString(PyExc_TypeError, "first and second arguments must be a sequence of players");
            return NULL;
        }

        int len=(int)PySequence_Length(pTeamSequence[t]);

        for (int i = 0; i < len; i++) { 
            PyObject *player = PySequence_GetItem(pTeamSequence[t], i);
            if (player == NULL){ // some failure
                PyErr_SetString(PyExc_TypeError, "unable to get player object from sequence");
                return NULL;
            }
            
            R2PlayerInfoObject *playerInfoObject= (R2PlayerInfoObject *)player;
            R2PlayerInfo playerInfo;
            setR2PlayerInfo(playerInfo, *playerInfoObject);
            team[t].push_back(playerInfo);
            Py_DECREF(player);
        }
    }

    simulations[handle]->setEnvironment(tick, score1, score2, rstate, ball, 
        team[0], team[1], (bool) lastTouchedTeam2, ballCatched, (bool) ballCatchedTeam2) ;

   Py_INCREF(Py_None);
   return Py_None;
}

// helper method for people using python prior to 3.7 because python<3.7 doesn't have true IEEE754 remainder (numpy neither)
// this should be equivalent to python 3.7: math.remainder(dividend, divisor)
static PyObject *robosoc2d_remainder(PyObject *self, PyObject *args, PyObject *keywds){
    static char *keywords[] = {(char *)"dividend", (char *)"divisor", NULL};
    double dividend,divisor;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "dd", keywords, &dividend, &divisor)){
        PyErr_SetString(PyExc_TypeError, "wrong parameters, they should be two floats");
        return NULL;
    }

    return  PyFloat_FromDouble(remainder(dividend, divisor));
}


static PyMethodDef Robosoc2dMethods[] = { 
    {"get_version", (PyCFunction)robosoc2d_getVersion, METH_NOARGS, "get_version()\n\nIt returns a string containing the version of the simulator.\nIt uses the format: \"major.minor.revision\". No parameters." },
    {"get_default_settings", (PyCFunction)robosoc2d_getDefaultSettings, METH_NOARGS, "get_default_settings()\n\nIt returns a settings object containing the default settings of simulator. No parameters." },
    {"get_seed_by_current_time", (PyCFunction)robosoc2d_getSeedByCurrentTime, METH_NOARGS, "get_seed_by_current_time()\n\nIt returns a random seed generated by current time. No parameters." },
    {"build_simulator", (PyCFunction)robosoc2d_buildSimulator, METH_VARARGS|METH_KEYWORDS, "build_simulator(team1, team2, team1name, team2name, random_seed, game_settings)\n\nIt creates a simulator and returns an integer that represents an handle to it. First and second parameters are mandatory and must be sequences of players, one sequence per team. It is possible to have a different number of players in each team. The other parameters are not mandatory. The third and fourth parameters are strings containing team names. Fifth parameter is an integer containing the random seed to be used to initialize the random engine (if this parameter is missing, a random seed will be generated depending on current time). Sixth parameter is a settings object in case you want to choose you own settings. (a player is an object that implements the method step(self, env, pitch, settings, team1, team2) that receives the information about the game and returns the choosen action as a tuple composed by one integer and three floats) " },
    {"build_simpleplayer_simulator", (PyCFunction)robosoc2d_buildSimplePlayerSimulator, METH_VARARGS|METH_KEYWORDS, "build_simpleplayer_simulator (team1, how_manysimpleplayers_team1, team2, how_manysimpleplayers_team1, team1name, team2name, random_seed, game_settings)\n\nIt creates a simulator and returns an integer that represents an handle to it. For each team it is possible to use the built-in class SimplePlayer for some players. The user may decide how many SimplePlayer agent each team may have: the first players of the team will be the SimplePlayers ones (if any), and the subsequent players will be the ones inserted in the team sequences (that may possibly be empty). Since the SimplePlayers will be the first players of the team, and since the first player plays in the goalkeeper role, if a team has at least a SimplePlayer, it means the the goalkeeper will be certainly a SimplePlater. Parameters 1-4 are mandatory. First parameter must be a sequence containing the players objects for the first team, and second parameter is a boolean determining how many SimplePlayers have to be added at the beginning of the team. Third and fourth parameters are the same for the second team. It is possible to have a different number of players in each team. The other parameters are not mandatory. The fifth and sixth parameters are strings containing team names. Seventh parameter is an integer containing the random seed to be used to initialize the random engine (if this parameter is missing, a random seed will be generated depending on current time). Eighth parameter is a settings object in case you want to choose you own settings.(a player is an object that implements the method step(self, env, pitch, settings, team1, team2) that receives the information about the game and returns the choosen action as a tuple composed by one integer and three floats) " },
    {"simulator_step_if_playing", (PyCFunction)robosoc2d_simulatorStepIfPlaying, METH_VARARGS|METH_KEYWORDS, "simulator_step_if_playing (handle)\n\nIt runs a step of the simulation, if the simulation is still playable and not terminated. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean that is false if the game was still playable." },
    {"simulator_play_game", (PyCFunction)robosoc2d_simulatorPlayGame, METH_VARARGS|METH_KEYWORDS, "simulator_play_game (handle)\n\nIt runs all the step of the simulation till the end. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean containing True if the game was played, throws an exception otherwise." },
    {"simulator_delete", (PyCFunction)robosoc2d_simulatorDelete, METH_VARARGS|METH_KEYWORDS, "simulator_delete (handle)\n\nIt deletes a simulator. It accepts only one parameter: an integer that is an handle to the simulation. It returns a boolean containing True if the simulation has been deleted, False otherwise." },
    {"simulator_delete_all", (PyCFunction)robosoc2d_simulatorDeleteAll, METH_NOARGS, "simulator_delete_all ()\n\nIt deletes all simulators. No parameters. It returns None" },
    {"simulator_is_valid", (PyCFunction)robosoc2d_simulatorIsValid, METH_VARARGS|METH_KEYWORDS,"simulator_is_valid (handle)\n\nIt checks if handle passed as parameter refers to an existing simulator, and returns a boolean accordly"},
    {"simulator_get_state_string", (PyCFunction)robosoc2d_simulatorGetStateString, METH_VARARGS|METH_KEYWORDS,"simulator_get_state_string (handle)\n\nIt returns the state string of simulator. It accepts only one parameter: an integer that is an handle to the simulation."},
    {"simulator_get_game_state", (PyCFunction)robosoc2d_simulatorGetGameState, METH_VARARGS|METH_KEYWORDS,"simulator_get_game_state (handle)\n\nIt returns the game state of the simulator. It accepts only one parameter: an integer that is an handle to the simulation. It returns a tuple containing 5 objects: the first is an environment object, the second is a pitch object, the third is a settings object, the fourth is a tuple of player_info objects containing the infromation about first team players, and the fifth object is a tuple of player_info for the secondo team."},
    {"simulator_get_random_seed", (PyCFunction)robosoc2d_simulatorGetRandomSeed, METH_VARARGS|METH_KEYWORDS,"simulator_get_random_seed (handle)\n\nIt returns the random seed of the simulator. It accepts only one parameter: an integer that is an handle to the simulation."},
    {"simulator_get_team_names", (PyCFunction)robosoc2d_simulatorGetTeamNames, METH_VARARGS|METH_KEYWORDS,"simulator_get_team_names (handle)\n\nIt returns the team names as a tuple containing two strings. It accepts only one parameter: an integer that is an handle to the simulation."},
    {"simulator_save_state_history", (PyCFunction)robosoc2d_simulatorSaveStateHistory, METH_VARARGS|METH_KEYWORDS,"simulator_save_state_history (handle, filename)\n\nIt saves the state history of the simulator. The first parameter is an integer that is an handle to the simulation. The second parameter is the file name. It returns a boolean representing success (True) or failure (False)."},
    {"simulator_save_actions_history", (PyCFunction)robosoc2d_simulatorSaveActionsHistory, METH_VARARGS|METH_KEYWORDS,"simulator_save_actions_history (handle, filename)\n\nIt saves the actions history of the simulator. The first parameter is an integer that is an handle to the simulation. The second parameter is the file name. It returns a boolean representing success (True) or failure (False)."},
    {"simulator_set_environment", (PyCFunction)robosoc2d_set_environment, METH_VARARGS|METH_KEYWORDS, "simulator_set_environment(handle, tick, score1, score2, state, ball_x, ball_y, ball_velocity_x, ball_velocity_y, team1, team2, last_touched_team2, ball_catched, ball_catched_team2) \n\n It sets the game configuration, making possible to decide things like players' and ball's positions, velocities and so on. The first parameter is an integer that is an handle to the simulation. The second parameter is an integer that represents the time tick. Third and fourth parameters are integers representing current score for the two teams. The fith parameter is an integer indicating the state of the game, to be picked up among the constants robosoc2d.STATE_* , for instance robosoc2d.STATE_PLAY . Next 4 parameters are floats containing ball position and velocity. Then the parameter team1 is a sequence containing objects of type robosoc2d.player_info that specify position, velocity and direction of the players of the first team. The parameter team2 is the same for the second team. Then there are 3 optional parameters: last_touched_team2 is a bool that indicates if the last team that touched the ball was team2. The integer ball_catched indicates if the ball is currently owned by a gall keeper: if >0 the ball is catched and only the goalkeeper can kick it and the ball moves with him, thenumber actually indicates for how many ticks the ball is still allowed to be possessed by the goalkeeper. The boolean ball_catched_team2 should be true if the goalkeeper that owns the ball is the one of second team."},
    {"remainder", (PyCFunction)robosoc2d_remainder, METH_VARARGS|METH_KEYWORDS,"remainder (dividend, divisor)\n\nMath remainder function following IEEE754 specification. Helper method for people using python prior to 3.7 because python<3.7 doesn't have true IEEE754 remainder (numpy neither). This should be equivalent to python 3.7: math.remainder(dividend, divisor)"},
    
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static void robosoc2d_exitModule(){
    isPythonActive=false;    
    //Python environment is already off at this point
    //each python object referenced/pointed by C++ object is not valid anymore (no matter what the reference count is)
    //and no Python API should be called at this point
    // see https://docs.python.org/3/c-api/sys.html#c.Py_AtExit
}


const char *robosoc2dmodule_doc="robosoc2d\n\n\
Very Simplified 2D Robotic Soccer Simulator (c) 2021 Ruggero Rossi\n\n\
Classes: \n\
settings \n\
pitch \n\
environment \n\
player_info \n\
error \n\
\n\
Functions: \n\
get_default_settings () \n\
get_seed_by_current_time () \n\
build_simulator (team1, team2, team1name, team2name, random_seed, game_settings) \n\
build_simpleplayer_simulator (team1, how_manysimpleplayers_team1, team2, how_manysimpleplayers_team1, team1name, team2name, random_seed, game_settings) \n\
simulator_step_if_playing (handle) \n\
simulator_play_game (handle) \n\
simulator_delete (handle) \n\
simulator_delete_all () \n\
simulator_is_valid (handle) \n\
simulator_get_state_string (handle) \n\
simulator_get_game_state (handle) \n\
simulator_get_random_seed (handle) \n\
simulator_get_team_names (handle) \n\
simulator_save_state_history (handle, filename) \n\
simulator_save_actions_history (handle, filename) \n\
remainder (dividend, divisor) \n\
\n\
Constants: \n\
STATE_INACTIVE \n\
STATE_READY \n\
STATE_KICKOFF1 \n\
STATE_KICKOFF2 \n\
STATE_PLAY \n\
STATE_STOPPED \n\
STATE_GOALKICK1UP \n\
STATE_GOALKICK1DOWN \n\
STATE_GOALKICK2UP \n\
STATE_GOALKICK2DOWN \n\
STATE_CORNER1UP \n\
STATE_CORNER1DOWN \n\
STATE_CORNER2UP \n\
STATE_CORNER2DOWN \n\
STATE_THROWIN1 \n\
STATE_THROWIN2 \n\
STATE_PAUSED \n\
STATE_HALFTIME \n\
STATE_GOAL1 \n\
STATE_GOAL2 \n\
STATE_ENDED \n\
ACTION_NOOP \n\
ACTION_MOVE \n\
ACTION_DASH \n\
ACTION_TURN \n\
ACTION_KICK \n\
ACTION_CATCH \n";

static struct PyModuleDef robosoc2dmodule = {
    PyModuleDef_HEAD_INIT,
    "robosoc2d",   /* name of module */
    robosoc2dmodule_doc,   //doc
    -1, // per interpreter size
    Robosoc2dMethods
};

PyMODINIT_FUNC
PyInit_robosoc2d(void) {
    R2SettingsType.tp_name = "robosoc2d.settings";
    R2SettingsType.tp_basicsize = sizeof(R2SettingsObject);
    R2SettingsType.tp_itemsize = 0;
    R2SettingsType.tp_flags = Py_TPFLAGS_DEFAULT;
    R2SettingsType.tp_doc = R2Settings_doc;
    R2SettingsType.tp_members = R2Settings_members;
    R2SettingsType.tp_methods = R2Settings_methods;
    R2SettingsType.tp_new = PyType_GenericNew;
    R2SettingsType.tp_repr = (reprfunc) R2SettingsObject_repr;
    R2SettingsType.tp_dealloc = (destructor) R2SettingsType_dealloc;    //unnecessary

    R2PitchType.tp_name = "robosoc2d.pitch";
    R2PitchType.tp_basicsize = sizeof(R2PitchObject);
    R2PitchType.tp_itemsize = 0;
    R2PitchType.tp_flags = Py_TPFLAGS_DEFAULT;
    R2PitchType.tp_doc = R2Pitch_doc;
    R2PitchType.tp_members = R2Pitch_members;
    R2PitchType.tp_methods = R2Pitch_methods;
    R2PitchType.tp_new = PyType_GenericNew;
    R2PitchType.tp_repr = (reprfunc) R2PitchObject_repr;
    R2PitchType.tp_dealloc = (destructor) R2PitchType_dealloc;  //unnecessary

    R2EnvironmentType.tp_name = "robosoc2d.environment";
    R2EnvironmentType.tp_basicsize = sizeof(R2EnvironmentObject);
    R2EnvironmentType.tp_itemsize = 0;
    R2EnvironmentType.tp_flags = Py_TPFLAGS_DEFAULT;
    R2EnvironmentType.tp_doc = R2Environment_doc;
    R2EnvironmentType.tp_members = R2Environment_members;
    R2EnvironmentType.tp_methods = R2Environment_methods;
    R2EnvironmentType.tp_new = PyType_GenericNew;
    R2EnvironmentType.tp_repr = (reprfunc) R2EnvironmentObject_repr;
    R2EnvironmentType.tp_dealloc = (destructor) R2EnvironmentType_dealloc;  //unnecessary

    R2PlayerInfoType.tp_name = "robosoc2d.player_info";
    R2PlayerInfoType.tp_basicsize = sizeof(R2PlayerInfoObject);
    R2PlayerInfoType.tp_itemsize = 0;
    R2PlayerInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    R2PlayerInfoType.tp_doc = R2PlayerInfo_doc;
    R2PlayerInfoType.tp_members = R2PlayerInfo_members;
    R2PlayerInfoType.tp_methods = R2PlayerInfo_methods;
    R2PlayerInfoType.tp_new = PyType_GenericNew;
    R2PlayerInfoType.tp_repr = (reprfunc) R2PlayerInfoObject_repr;
    R2PlayerInfoType.tp_dealloc = (destructor) R2PlayerInfoType_dealloc;    //unnecessary


    if (PyType_Ready(&R2SettingsType) < 0)
    {
        return NULL;
    }

    if (PyType_Ready(&R2PitchType) < 0)
    {
        return NULL;
    }

    if (PyType_Ready(&R2EnvironmentType) < 0)
    {
        return NULL;
    }

    if (PyType_Ready(&R2PlayerInfoType) < 0)
    {
        return NULL;
    }

    PyObject* m = PyModule_Create(&robosoc2dmodule);
    if (m == NULL) {
        return NULL;
    }

    R2Error = PyErr_NewException("robosoc2d.error", NULL, NULL);
    Py_XINCREF(R2Error);
    if (PyModule_AddObject(m, "error", R2Error) < 0) {
        Py_XDECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&R2SettingsType);
    if (PyModule_AddObject(m, "settings", (PyObject *) &R2SettingsType) < 0) {
        Py_DECREF(&R2SettingsType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&R2PitchType);
    if (PyModule_AddObject(m, "pitch", (PyObject *) &R2PitchType) < 0) {
        Py_DECREF(&R2SettingsType);
        Py_DECREF(&R2PitchType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&R2EnvironmentType);
    if (PyModule_AddObject(m, "environment", (PyObject *) &R2EnvironmentType) < 0) {
        Py_DECREF(&R2SettingsType);
        Py_DECREF(&R2PitchType);
        Py_DECREF(&R2EnvironmentType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&R2PlayerInfoType);
    if (PyModule_AddObject(m, "player_info", (PyObject *) &R2PlayerInfoType) < 0) {
        Py_DECREF(&R2SettingsType);
        Py_DECREF(&R2PitchType);
        Py_DECREF(&R2EnvironmentType);
        Py_DECREF(&R2PlayerInfoType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }

    stepMethodName =  PyUnicode_FromString("step"); 
    if(stepMethodName==NULL)
    {
        Py_DECREF(&R2SettingsType);
        Py_DECREF(&R2PitchType);
        Py_DECREF(&R2EnvironmentType);
        Py_DECREF(&R2PlayerInfoType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(stepMethodName);

    if (
	    PyModule_AddIntConstant(m, "STATE_INACTIVE", static_cast<int>(R2State::Inactive)) ||
	    PyModule_AddIntConstant(m, "STATE_READY", static_cast<int>(R2State::Ready)) ||
	    PyModule_AddIntConstant(m, "STATE_KICKOFF1", static_cast<int>(R2State::Kickoff1)) ||
	    PyModule_AddIntConstant(m, "STATE_KICKOFF2", static_cast<int>(R2State::Kickoff2)) ||
	    PyModule_AddIntConstant(m, "STATE_PLAY", static_cast<int>(R2State::Play)) ||
	    PyModule_AddIntConstant(m, "STATE_STOPPED", static_cast<int>(R2State::Stopped)) ||
	    PyModule_AddIntConstant(m, "STATE_GOALKICK1UP", static_cast<int>(R2State::Goalkick1up)) ||
	    PyModule_AddIntConstant(m, "STATE_GOALKICK1DOWN", static_cast<int>(R2State::Goalkick1down)) ||
	    PyModule_AddIntConstant(m, "STATE_GOALKICK2UP", static_cast<int>(R2State::Goalkick2up)) ||
	    PyModule_AddIntConstant(m, "STATE_GOALKICK2DOWN", static_cast<int>(R2State::Goalkick2down)) ||
	    PyModule_AddIntConstant(m, "STATE_CORNER1UP", static_cast<int>(R2State::Corner1up)) ||
	    PyModule_AddIntConstant(m, "STATE_CORNER1DOWN", static_cast<int>(R2State::Corner1down)) ||
	    PyModule_AddIntConstant(m, "STATE_CORNER2UP", static_cast<int>(R2State::Corner2up)) ||
	    PyModule_AddIntConstant(m, "STATE_CORNER2DOWN", static_cast<int>(R2State::Corner2down)) ||
	    PyModule_AddIntConstant(m, "STATE_THROWIN1", static_cast<int>(R2State::Throwin1)) ||
	    PyModule_AddIntConstant(m, "STATE_THROWIN2", static_cast<int>(R2State::Throwin2)) ||
	    PyModule_AddIntConstant(m, "STATE_PAUSED", static_cast<int>(R2State::Paused)) ||
	    PyModule_AddIntConstant(m, "STATE_HALFTIME", static_cast<int>(R2State::Halftime)) ||
	    PyModule_AddIntConstant(m, "STATE_GOAL1", static_cast<int>(R2State::Goal1)) ||
	    PyModule_AddIntConstant(m, "STATE_GOAL2", static_cast<int>(R2State::Goal2)) ||
	    PyModule_AddIntConstant(m, "STATE_ENDED", static_cast<int>(R2State::Ended)) ||

	    PyModule_AddIntConstant(m, "ACTION_NOOP", static_cast<int>(R2ActionType::NoOp)) ||
	    PyModule_AddIntConstant(m, "ACTION_MOVE", static_cast<int>(R2ActionType::Move)) ||
	    PyModule_AddIntConstant(m, "ACTION_DASH", static_cast<int>(R2ActionType::Dash)) ||
	    PyModule_AddIntConstant(m, "ACTION_TURN", static_cast<int>(R2ActionType::Turn)) ||
	    PyModule_AddIntConstant(m, "ACTION_KICK", static_cast<int>(R2ActionType::Kick)) ||
	    PyModule_AddIntConstant(m, "ACTION_CATCH", static_cast<int>(R2ActionType::Catch)) 
	) {
        Py_DECREF(stepMethodName);
        Py_DECREF(&R2SettingsType);
        Py_DECREF(&R2PitchType);
        Py_DECREF(&R2EnvironmentType);
        Py_DECREF(&R2PlayerInfoType);
        Py_DECREF(R2Error);
        Py_CLEAR(R2Error);
        Py_DECREF(m);
        return NULL;
    }

    Py_AtExit(robosoc2d_exitModule);

    isPythonActive=true;

    return m;
}

