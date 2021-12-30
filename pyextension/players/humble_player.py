# (c) 2021 Ruggero Rossi
# a simple, reactive, robosoc2d player agent (with no planning)
# porting to Python of simple_player.cpp
import robosoc2d as r2s
import math
import numpy as np
import random

R2SMALL_EPSILON = np.finfo(float).eps
R2EPSILON = R2SMALL_EPSILON*10.0

PASS_GOALKEEPER_DISADVANTAGE=-10
PASS_BACKWARD_DISADVANTAGE=-5
PASS_ADVANCED_ADVANTAGE=5
PASS_TRAJECTORY_OCCLUDED_DISADVANTAGE=-20
PASS_FAVOURITE_DISTANCE=10
PASS_DISTANCE_DISADVANTAGE=-0.1
ADVANCED_PASS_LENGTH=5.0
OUT_OF_ANGLE_DISADVANTAGE=-25
OWN_GOAL_DISTANCE_THRESHOLD=6
OWN_GOAL_DISTANCE_DISADVANTAGE=-15

CHECKED_DISTANCE=3.0
N_ADVANCING_INTERVALS=6
ADVANCING_INCENTIVE=1.0
AVOID_OUT_INCENTIVE=0.5
DANGER_BORDER_RATIO=0.9
GOAL_INCENTIVE =50.0

MINIMUM_PASS_VALUE=0.1
SCORE_DISTANCE=8.0 # closer than this to goal => kick and try to score
MARK_DISTANCE=1.0
GOALKEEPER_INTERVENTION_DISTANCE=3.0

#sets angle between 0 and 2 PI
def _fix_angle_positive(angle):
    #angle2 = math.remainder(angle , 2*np.pi) #you can replace next line with this only from python 3.7 onwards
    angle2 = r2s.remainder(angle , 2*np.pi) 
    if angle2 < 0.0:
        angle2 = 2*np.pi + angle2
    return angle2

#sets angle between + and - PI
def _fix_angle_two_sides(angle):
    #return math.remainder(angle , 2*np.pi) #you can replace next line with this only from python 3.7 onwards
    return r2s.remainder(angle , 2*np.pi) 

# explanation: http://paulbourke.net/geometry/pointlineplane/
# p is the point - 2d point ndarray
# s1 and s2 are the segment's ends - 2d points ndarray
def _distance_point_segment(p, s1, s2):
    d=s2-s1
    dp1=p-s1

    #if the segment is collapsed in one point we can't use the formula but we can still calculate point to point distance
    if (d[0] >= -R2SMALL_EPSILON) and (d[0] <= R2SMALL_EPSILON) and (d[1] >= -R2SMALL_EPSILON) and (d[1] <= R2SMALL_EPSILON):
        return np.linalg.norm(dp1).item()
    
    l = np.linalg.norm(d)
    u = np.dot(dp1, d)/(l*l)
    if (u<0.0) or (u>1.0):
        dp2=p-s2
        return min(np.linalg.norm(dp1).item(), np.linalg.norm(dp2).item())
    intersection = s1 + d*u
    return np.linalg.norm(p-intersection).item()


class HumblePlayer:
    def __init__(self, team, shirt_number):
        self.team = team
        self.shirt_number = shirt_number
        self.prev_state = r2s.STATE_INACTIVE
        self.prev_action = (r2s.ACTION_NOOP, 0.0, 0.0, 0.0)
        #self.env = None
        #self.pitch = None
        #self.settings = None
        #self.teams = None
        random.seed(42)

    def _transform_action_if_necessary(self, action):
        if ( (self.team==1) and (not self.env.halftime_passed) ) or ( (self.team==0) and self.env.halftime_passed ) :
            if action[0] == r2s.ACTION_MOVE :
                    action[1] = -action[1]
                    action[2] = -action[2]
                    action[3] = _fix_angle_positive(action[3] + np.pi)
            elif (action[0] == r2s.ACTION_DASH) or (action[0] == r2s.ACTION_TURN) or (action[0] == r2s.ACTION_KICK) or (action[0] == r2s.ACTION_CATCH):
                    action[1] = _fix_angle_positive(action[1] + np.pi)

        return action
        

    def _transform_env_if_necessary(self):
        self.env = self.normal_env.copy()
        if self.team==1:
            self.teams[0] = self.normal_teams[1]
            self.teams[1] = self.normal_teams[0]
            self.env.score1 = self.normal_env.score2
            self.env.score2 = self.normal_env.score1
            self.env.last_touched_team2 = not self.env.last_touched_team2 # teams[1] thinks to be teams[0]
            self.env.ball_catched_team2 = not self.env.ball_catched_team2
            # now invert the state
            state=self.env.state
            if state == r2s.STATE_KICKOFF1:
                self.env.state = r2s.STATE_KICKOFF2
            elif state == r2s.STATE_KICKOFF2 :
                self.env.state = r2s.STATE_KICKOFF1
            elif state == r2s.STATE_GOALKICK1UP :
                self.env.state = r2s.STATE_GOALKICK2DOWN
            elif state == r2s.STATE_GOALKICK1DOWN :
                self.env.state = r2s.STATE_GOALKICK2UP
            elif state == r2s.STATE_GOALKICK2UP :
                self.env.state = r2s.STATE_GOALKICK1DOWN
            elif state == r2s.STATE_GOALKICK2DOWN :
                self.env.state = r2s.STATE_GOALKICK1UP
            elif state == r2s.STATE_CORNER1UP :
                self.env.state = r2s.STATE_CORNER2DOWN
            elif state == r2s.STATE_CORNER1DOWN :
                self.env.state = r2s.STATE_CORNER2UP
            elif state == r2s.STATE_CORNER2UP :
                self.env.state = r2s.STATE_CORNER1DOWN
            elif state == r2s.STATE_CORNER2DOWN :
                self.env.state = r2s.STATE_CORNER1UP
            elif state == r2s.STATE_THROWIN1 :
                self.env.state = r2s.STATE_THROWIN2
            elif state == r2s.STATE_THROWIN2 :
                self.env.state = r2s.STATE_THROWIN1
            elif state == r2s.STATE_GOAL1 :
                self.env.state = r2s.STATE_GOAL2
            elif state == r2s.STATE_GOAL2 :
                self.env.state = r2s.STATE_GOAL1

        if self.env.halftime_passed :
            state = self.env.state
            if state== r2s.STATE_GOALKICK1UP :
                self.env.state = r2s.STATE_GOALKICK1DOWN 
            elif state== r2s.STATE_GOALKICK1DOWN :
                self.env.state = r2s.STATE_GOALKICK1UP 
            elif state== r2s.STATE_GOALKICK2UP :
                self.env.state = r2s.STATE_GOALKICK2DOWN 
            elif state== r2s.STATE_GOALKICK2DOWN :
                self.env.state = r2s.STATE_GOALKICK2UP 
            elif state== r2s.STATE_CORNER1UP :
                self.env.state = r2s.STATE_CORNER1DOWN 
            elif state== r2s.STATE_CORNER1DOWN :
                    self.env.state = r2s.STATE_CORNER1UP 
            elif state== r2s.STATE_CORNER2UP :
                    self.env.state = r2s.STATE_CORNER2DOWN 
            elif state== r2s.STATE_CORNER2DOWN :
                    self.env.state = r2s.STATE_CORNER2UP 

        if ( (self.team==1) and (not self.env.halftime_passed) ) or ( (self.team==0) and self.env.halftime_passed):
            self.env.ball_x = - self.env.ball_x
            self.env.ball_y = - self.env.ball_y
            self.env.ball_velocity_x = - self.env.ball_velocity_x
            self.env.ball_velocity_y = - self.env.ball_velocity_y

            for t in (0,1):
                for player in self.teams[t]:
                    player.x = -player.x
                    player.y = -player.y
                    player.velocity_x = - player.velocity_x
                    player.velocity_y = - player.velocity_y
                    player.direction = _fix_angle_positive(player.direction + np.pi)

    def _is_team_resume_kicking(self):
        return (self.prev_state==r2s.STATE_KICKOFF1) or (self.prev_state==r2s.STATE_GOALKICK1UP) or (self.prev_state==r2s.STATE_GOALKICK1DOWN) or (self.prev_state==r2s.STATE_THROWIN1) or (self.prev_state==r2s.STATE_CORNER1UP) or (self.prev_state==r2s.STATE_CORNER1DOWN) 

    def _best_kick_angle_possible(self, angle):
        best_angle=angle
        if(not self._is_team_resume_kicking()) and (not self.settings.simplified):
            p=self.teams[0][self.shirt_number]
            a=_fix_angle_positive(angle)
            d=_fix_angle_two_sides(a-p.direction)
            if d > self.settings.kickable_direction_angle :
                best_angle=p.direction+self.settings.kickable_direction_angle   
            elif d < -self.settings.kickable_direction_angle:
                best_angle=p.direction-self.settings.kickable_direction_angle
        return best_angle

    def _is_kick_oriented_toward_own_goal(self, angle):
        a=_fix_angle_positive(angle)
        if  (a< (np.pi/2.0) ) or (a> (np.pi*1.5) ):
            return (False, 0.0)
        dir=np.array([math.cos(a), math.sin(a)])
        if dir[0]==0.0:
            return (False, 0.0)
        
        y=self.env.ball_y+ (self.pitch.x2- self.env.ball_x)*dir[1]/dir[0]
        if (y<self.pitch.y_goal1) and (y>self.pitch.y_goal2) :
            return (True, y)
        return (False, 0.0)

    def _angle_toward_point(self, x, y):
        p= self.teams[0][self.shirt_number]
        dx= x - p.x
        dy= y - p.y
        if (dx==0.0) and (dy==0.0):
            return 0.0
        return math.atan2(dy,dx)

    def _angle_toward_ball(self):
        return self._angle_toward_point(self.env.ball_x, self.env.ball_y)

    def _angle_toward_center_goal(self):
        return self._angle_toward_point(self.pitch.x1, 0.0)

    def _angle_toward_upper_pole(self):
        return self._angle_toward_point(self.pitch.x1, self.pitch.y_goal1)
    
    def _angle_toward_lower_pole(self):
        return self._angle_toward_point(self.pitch.x1, self.pitch.y_goal2)
    
    def _angle_toward_own_upper_pole(self):
        return self._angle_toward_point(self.pitch.x2, self.pitch.y_goal1)

    def _angle_toward_own_lower_pole(self):
        return self._angle_toward_point(self.pitch.x2, self.pitch.y_goal2)
    
    def _ball_angle_toward_point(self, x, y):
        dx=x - self.env.ball_x
        dy=y - self.env.ball_y
        if (dx==0.0) and (dy==0.0):
            return 0.0
        return math.atan2(dy,dx)

    def _ball_angle_toward_center_goal(self):
        return self._ball_angle_toward_point(self.pitch.x1, 0.0)

    def _is_ball_kickable(self):
        p= self.teams[0][self.shirt_number] 
        dx = self.env.ball_x - p.x
        dy = self.env.ball_y - p.y
        dist = math.sqrt(dx*dx + dy*dy)

        # is ball reachable
        if dist > self.settings.kickable_distance:
            return False

        # is ball in front of player? only when not in kicking from restart game.
        if (not self.settings.simplified) and (not self._is_team_resume_kicking() ) and (dist > R2SMALL_EPSILON):
            cosinus_player_ball= (dx*math.cos(p.direction) + dy*math.sin(p.direction))/dist
            if cosinus_player_ball <= self.cos_kickable_angle:  
                return False

        return True
    
    # not considering the goalkeeper
    def _opponent_baricenter(self):
        sum_x=0.0
        sum_y=0.0
        for i in range(1, len(self.teams[1]) ):
            sum_x+=self.teams[1][i].x
            sum_y+=self.teams[1][i].y
        return np.array([sum_x,sum_y])*(1/i)

    # except goalkeeper
    def _is_opponent_closer_to_ball(self):
        closer_team=0
        min_dist=1000000
        for t in (0,1):
            for i in range(1, len(self.teams[t]) ):
                dx = self.teams[t][i].x - self.env.ball_x
                dy = self.teams[t][i].y - self.env.ball_y
                dist = dx*dx + dy*dy
                if dist<min_dist :
                    closer_team=t
                    min_dist=dist
        return closer_team==1

    # except goalkeeper
    def _player_closest_to_ball(self, team):
        closest = 1
        if len(team) < 2:
            return 0
        dx= team[1].x - self.env.ball_x
        dy= team[1].y - self.env.ball_y
        min_dist = dx*dx + dy*dy
        for i in range(2, len(team) ):
            dx= team[i].x - self.env.ball_x
            dy= team[i].y - self.env.ball_y
            dist = dx*dx + dy*dy

            if dist<min_dist :
                closest=i
                min_dist=dist
            
        return closest

    # except goalkeeper
    def _my_player_closest_to_ball(self):
        return self._player_closest_to_ball(self.teams[0])
    
    # except goalkeeper
    def _opponent_player_closest_to_ball(self):
        return self._player_closest_to_ball(self.teams[1])
    
    #be careful: returns -1 if opposing team is empty
    #currently unused method
    def _my_closest_opponent(self):
        if len(self.teams[1]) < 1 :
            return -1
        myself=self.teams[0][self.shirt_number]
        closest =0
        dx = self.teams[1][0].x - myself.x
        dy = self.teams[1][0].y - myself.y
        min_dist = dx*dx + dy*dy

        for i in range(1, len(self.teams[1]) ):
            dx = self.teams[1][i].x - myself.x
            dy = self.teams[1][i].y - myself.y
            dist = dx*dx + dy*dy

            if dist < min_dist :
                closest=i
                min_dist=dist
            
        return closest
    
    def _my_player_closest_to_opponent(self, opponent, assigned):
        pos_x = self.teams[1][opponent].x
        pos_y = self.teams[1][opponent].y
        closest =-1
        min_dist= 1000000
        for i in range(len(assigned)):
            if(assigned[i]==0):
                dx = pos_x - self.teams[0][i].x
                dy = pos_y - self.teams[0][i].y
                dist = dx*dx + dy*dy
                if dist<min_dist:
                    closest=i
                    min_dist=dist
        return closest
    
    #s1 and s2, 2d points, ndarray
    def _min_opponent_segment_distance(self, s1, s2):
        min_segment_distance=1000.0
        for i in range(0, len(self.teams[1]) ):
            p2=np.array([self.teams[1][i].x, self.teams[1][i].y])
            segment_distance = _distance_point_segment(p2, s1, s2)
            min_segment_distance=min(min_segment_distance,segment_distance)
        return min_segment_distance

    def _calc_advanced_position(self, player):
        pos= np.array([self.teams[0][player].x, self.teams[0][player].y])
        d= np.array([self.pitch.x1, 0.0]) - pos
        d/= np.linalg.norm(d) #normalized
        d*=ADVANCED_PASS_LENGTH # scaled. segment from player to center of opposite goal
        pos+=d
        return pos

    def _companion_pass_value(self, player, advanced):
        value=0.0
        pos =np.array([self.teams[0][player].x, self.teams[0][player].y])
        if advanced:
            pos=self._calc_advanced_position(player)
            value+= PASS_ADVANCED_ADVANTAGE

        ball=np.array([self.env.ball_x, self.env.ball_y])
        min_segment_distance=self._min_opponent_segment_distance(pos, ball)
        
        if min_segment_distance<= (self.settings.ball_radius+self.settings.player_radius):
            value+= PASS_TRAJECTORY_OCCLUDED_DISADVANTAGE

        if player==0 :
            value+= PASS_GOALKEEPER_DISADVANTAGE

        if pos[0] < self.env.ball_x:
            value+=PASS_BACKWARD_DISADVANTAGE

        dist= np.linalg.norm(ball - pos)
        value+=math.fabs(PASS_FAVOURITE_DISTANCE-dist)*PASS_DISTANCE_DISADVANTAGE

        # calc if kicking angle is possible
        out_of_angle=False
        d=pos - ball
        angle=0.0
        if (not self.settings.simplified) and (d[0]!=0.0) and (d[1]!=0.0):
            angle=math.atan2(d[1],d[0])
            if not self._is_team_resume_kicking():
                p=self.teams[0][self.shirt_number]
                a=_fix_angle_positive(angle)
                diff=_fix_angle_two_sides(a-p.direction)
                if diff > self.settings.kickable_direction_angle:
                    value+=OUT_OF_ANGLE_DISADVANTAGE  
                    out_of_angle=True; 
                
                elif diff < -self.settings.kickable_direction_angle:
                    value+=OUT_OF_ANGLE_DISADVANTAGE;   
                    out_of_angle=True; 

        towards_own_goal, towards_y =self._is_kick_oriented_toward_own_goal(angle)
        if towards_own_goal:
            if out_of_angle:
                angle=self._best_kick_angle_possible(angle)
            
            own_goal=np.array([self.pitch.x2, towards_y])
            dist_goal= np.linalg.norm(ball-own_goal)
            if dist_goal< OWN_GOAL_DISTANCE_THRESHOLD:
                disadvantage= (OWN_GOAL_DISTANCE_THRESHOLD - dist_goal)*OWN_GOAL_DISTANCE_DISADVANTAGE/OWN_GOAL_DISTANCE_THRESHOLD
                value+=disadvantage
            
        return value
    

    def _choose_pass(self):
        nplayers=len(self.teams[0])
        best_still=-1000.0
        best_still_index=self.shirt_number
        best_advanced=-1000.0
        best_advanced_index=self.shirt_number
        for i in range(nplayers):
            if i!=self.shirt_number:
                value_still=self._companion_pass_value(i, False)
                if(best_still<value_still):
                    best_still=value_still
                    best_still_index=i
                
                value_advanced=self._companion_pass_value(i, True)
                if best_advanced<value_advanced:
                    best_advanced=value_advanced
                    best_advanced_index=i
                
        if best_still_index==self.shirt_number:
            return (np.array([0.0, 0.0]), best_still)
        
        if best_still > best_advanced:
            return ( np.array([self.teams[0][best_still_index].x, self.teams[0][best_still_index].y]), best_still)
        
        return (self._calc_advanced_position(best_advanced_index) , best_advanced)

    def _choose_best_throwin(self):
        pos, pass_value= self._choose_pass()
        return [r2s.ACTION_KICK,
                self._best_kick_angle_possible(self._ball_angle_toward_point(pos[0],pos[1])),
                self.settings.max_kick_power,
                0.0]           
    
    #this may be improved for corner kick case instead of reusing the throw-in method
    def _choose_best_corner_kick(self):
        return self._choose_best_throwin()

    def _choose_best_attack_kick(self):
        # check the best trajectory to kick the ball and do it
        n_intervals=12
        best_y=0.0
        best_value=0.0
        for i in range(n_intervals):
            py=self.pitch.y_goal1*(float(i)/n_intervals) + self.pitch.y_goal2*(float(n_intervals-i)/n_intervals)
            dist_value=self._min_opponent_segment_distance( np.array([self.env.ball_x, self.env.ball_y]),
                                                            np.array([self.pitch.x1, py]) )
            if(dist_value>best_value):
                best_value=dist_value
                best_y=py
            
        # not assured that actually you can kick that angle ! hence using _best_kick_angle_possible()
        return [r2s.ACTION_KICK,
                self._best_kick_angle_possible(self._ball_angle_toward_point(self.pitch.x1, best_y)),
                self.settings.max_kick_power,
                0.0]           

    def _choose_best_personal_ball_advancing(self):
        p=self.teams[0][self.shirt_number]
        angle_a=p.direction-self.settings.kickable_angle
        angle_b=p.direction+self.settings.kickable_angle
        best_value=0.0
        best_angle=0.0
        ball=np.array([self.env.ball_x, self.env.ball_y])
        goal_center=np.array([self.pitch.x1, 0.0])
        for i in range(N_ADVANCING_INTERVALS):
            angle=angle_a*(float(i)/N_ADVANCING_INTERVALS) + angle_b*(float(N_ADVANCING_INTERVALS-i)/N_ADVANCING_INTERVALS)
            target=np.array([math.cos(angle), math.sin(angle)])
            target*=CHECKED_DISTANCE
            target += ball
            value=self._min_opponent_segment_distance(ball, target)
            if target[0] > ball[0]:
                value+=ADVANCING_INCENTIVE
            danger_border=self.pitch.y1*DANGER_BORDER_RATIO
            if target[1] > danger_border:
                value+=-(target[1]-danger_border)/(self.pitch.y1*(1.0-DANGER_BORDER_RATIO))
            elif target[1] < -danger_border:
                value+=(target[1]-danger_border)/(self.pitch.y1*(1.0-DANGER_BORDER_RATIO))
            goal_closeness=GOAL_INCENTIVE*1.0/(1.0 + np.linalg.norm(goal_center-target) )
            goal_factor=goal_closeness*goal_closeness
            value+=goal_factor

            towards_own_goal, towards_y = self._is_kick_oriented_toward_own_goal(angle)
            if towards_own_goal:
                own_goal=np.array([self.pitch.x2, towards_y])
                dist_goal=np.linalg.norm(ball-own_goal)
                if dist_goal< OWN_GOAL_DISTANCE_THRESHOLD:
                    disadvantage= (OWN_GOAL_DISTANCE_THRESHOLD - dist_goal)*OWN_GOAL_DISTANCE_DISADVANTAGE/OWN_GOAL_DISTANCE_THRESHOLD
                    value+=disadvantage
                
            if value>best_value:
                best_value=value
                best_angle=angle

        return [r2s.ACTION_KICK,
                self._best_kick_angle_possible(best_angle),
                self.settings.max_kick_power/2.0,
                0.0]  
    
    def _choose_best_pass_or_advancing(self):
        action=None
        pos, pass_value= self._choose_pass()
        if pass_value>MINIMUM_PASS_VALUE:
            # pass the ball
            action= [r2s.ACTION_KICK,
                self._best_kick_angle_possible(self._ball_angle_toward_point(pos[0],pos[1])),
                self.settings.max_kick_power,
                0.0]         
        else:
            action= self._choose_best_personal_ball_advancing()
        
        return action
    
    def _play(self):
        action=None
        if self._is_ball_kickable():
            if self.prev_state==r2s.STATE_THROWIN1:
                action= self._choose_best_throwin()
            elif(self.prev_state==r2s.STATE_CORNER1DOWN) or (self.prev_state==r2s.STATE_CORNER1UP):
                action= self._choose_best_corner_kick()
            else:
                goal_dist=np.linalg.norm( np.array([self.env.ball_x - self.pitch.x1, self.env.ball_y]) )
                if goal_dist <= SCORE_DISTANCE:
                    action= self._choose_best_attack_kick()
                else:
                    action=self._choose_best_pass_or_advancing()
        else:
            if self._my_player_closest_to_ball()==self.shirt_number:  #reach for the ball
                action= [r2s.ACTION_DASH,
                         self._angle_toward_point(self.env.ball_x, self.env.ball_y),
                         self.settings.max_dash_power,
                         0.0] 
            else:  # TODO decide if going back in defence, stay or advance. If out of the pitch, come back
                pos=np.array([self.teams[0][self.shirt_number].x, self.teams[0][self.shirt_number].y])
                if pos[0] > self.pitch.x1:
                    if pos[1] < self.pitch.y2:   # out bottom right
                        action= [r2s.ACTION_DASH,
                                 np.pi*0.75,
                                 self.settings.max_dash_power,
                                 0.0] 
                    elif pos[1] > self.pitch.y1:  # out top right
                        action= [r2s.ACTION_DASH,
                                 np.pi*1.25,
                                 self.settings.max_dash_power,
                                 0.0] 
                    else:   #out right
                        action= [r2s.ACTION_DASH,
                                 np.pi,
                                 self.settings.max_dash_power,
                                 0.0] 
                elif pos[0] < self.pitch.x2:
                    if pos[1] < self.pitch.y2:   # out bottom left
                        action= [r2s.ACTION_DASH,
                                 np.pi*0.25,
                                 self.settings.max_dash_power,
                                 0.0] 
                    elif pos[1] > self.pitch.y1: # out top left
                        action= [r2s.ACTION_DASH,
                                 np.pi*1.75,
                                 self.settings.max_dash_power,
                                 0.0] 
                    else:   #out left
                        action= [r2s.ACTION_DASH,
                                 0.0,
                                 self.settings.max_dash_power,
                                 0.0] 
                elif pos[1] > self.pitch.y1:  # out top
                    action= [r2s.ACTION_DASH,
                                 np.pi*1.5,
                                 self.settings.max_dash_power,
                                 0.0] 
                elif pos[1] < self.pitch.y2 :  # out bottom
                    action= [r2s.ACTION_DASH,
                                 np.pi*0.5,
                                 self.settings.max_dash_power,
                                 0.0] 
                else:   # try to find your position
                    if self.shirt_number!=0:
                        action= self._position_player()
                    else:
                        action= self._position_goalkeeper()
                
        return action

    def _position_player(self):
        action=None

        if self._is_opponent_closer_to_ball():  #assume opponents are attacking
            if (self.env.ball_x < self.go_back_threshold_distance) or np.linalg.norm( self._opponent_baricenter() - np.array( [self.pitch.x2,0.0]) ) > self.go_back_baricenter_threshold_distance:
                # assign an opponent to mark. 0 is unassigned (you don't mark the goalkeepr), -1 is ball or no opponent. Goalkeeper is not marking, opponent goalkeeper is not marked
                assignments=[0]*len(self.teams[0]) 
                busy=self._my_player_closest_to_ball()    # this one is busy reaching for the ball
                assignments[busy]=-1
                assignments[0]=-1  #goalkeeper
                busy_opponent = self._opponent_player_closest_to_ball()
                for i in range(1, len(self.teams[1])):
                    if i!=busy_opponent:
                        p= self._my_player_closest_to_opponent(i, assignments)
                        if p>0 : # not assigning 0 (goal keeper) and -1 (no one)
                            assignments[p]=i
                    
                if assignments[self.shirt_number]< 0:   # not assigned (shouldn't be here if same number of players): reach for ball
                    action= [r2s.ACTION_DASH,
                                 self._angle_toward_ball(),
                                 self.settings.max_dash_power,
                                 0.0] 
                elif assignments[self.shirt_number] > 0:   # mark a player
                    target=np.array([self.teams[1][assignments[self.shirt_number]].x , self.teams[1][assignments[self.shirt_number]].y])
                    delta= np.array([self.pitch.x2,0.0])-target
                    delta/=np.linalg.norm(delta) 
                    delta*=MARK_DISTANCE
                    target +=delta
                    action= [r2s.ACTION_DASH,
                                 self._angle_toward_point(target[0], target[1]),
                                 self.settings.max_dash_power,
                                 0.0]    
            else:
                pos=np.array([self.teams[0][self.shirt_number].x, self.teams[0][self.shirt_number].y])
                angle=0.0
                if pos[1] >0.0:
                    angle=self._angle_toward_own_upper_pole()
                else:
                    angle=self._angle_toward_own_lower_pole()
                action= [r2s.ACTION_DASH,
                                 angle,
                                 self.settings.max_dash_power,
                                 0.0] 
        else:
            pos=np.array([self.teams[0][self.shirt_number].x, self.teams[0][self.shirt_number].y])
            if pos[0] < self.run_forward_threshold_distance:
                angle=0.0
                if pos[1] >0.0:
                    angle= self._angle_toward_upper_pole()
                else:
                    angle= self._angle_toward_lower_pole()
                action= [r2s.ACTION_DASH,
                                 angle,
                                 self.settings.max_dash_power,
                                 0.0] 
            
            else:   # try to stay in an unmarked position moving randomly
                angle=  random.uniform(0.0, 2*np.pi)
                action= [r2s.ACTION_DASH,
                                 angle,
                                 self.settings.max_dash_power,
                                 0.0]  
            
        return action
    
    def _position_goalkeeper(self):
        action=None
        p= self.teams[0][self.shirt_number]
        pos=np.array([p.x, p.y])

        # if ball close, go for it
        ball=np.array([self.env.ball_x, self.env.ball_y])

        if ( np.linalg.norm(ball-pos) < GOALKEEPER_INTERVENTION_DISTANCE) and (p.x < 0.0) :
            action= [r2s.ACTION_DASH,
                                 self._angle_toward_ball(),
                                 self.settings.max_dash_power,
                                 0.0]   
        else:
            goal_center=np.array([self.pitch.x2, 0.0])
            line_ball_goal=ball-goal_center
            wanna_x= line_ball_goal[0]/3.0
            wanna_y= line_ball_goal[1]/3.0
            wanna_be=np.array([self.pitch.x2+wanna_x, wanna_y])
            pos_diff=wanna_be-pos

            if np.linalg.norm(pos_diff) > 0.5 :
                angle=0.0
                if pos_diff[0] == 0.0 :
                    if pos_diff[1]>0.0 :
                        angle=np.pi/2.0
                    else:
                        angle=-np.pi/2.0
                else :
                    angle=math.atan2(pos_diff[1], pos_diff[0])
                                
                action= [r2s.ACTION_DASH,
                                 angle,
                                 self.settings.max_dash_power,
                                 0.0]   
            
            else:   # if not moving, face the ball
                action= [r2s.ACTION_DASH,
                                 self._angle_toward_ball(),
                                 0.0,
                                 0.0]  
            
        return action

    def _play_goalkeeper(self):
        action=None
        # 1) if ball is close and moving towards goal, catch it
        # 2) if ball is close and slow or still kick it to a companion (THE ONE THAT IS FAR FROM)
        # 3) otherwise goalkeeper wants to place himself on the line between the ball and the center of the goal
        #      choosing the distance depending on the most advanced opponent and on the ball
        p= self.teams[0][self.shirt_number]
        pos=np.array([p.x, p.y])
        ball=np.array([self.env.ball_x, self.env.ball_y])

        d= ball-pos
        dist= np.linalg.norm(d)

        cosinus_player_ball= (d[0]*math.cos(p.direction) + d[1]*math.sin(p.direction))/dist

        if (dist<self.settings.kickable_distance) and (cosinus_player_ball >= self.cos_kickable_angle) :
            action= self._choose_best_pass_or_advancing();    # actually if goalkeeper is catching the ball, this function is not accurate because the real ball position at the moment of kicking would not be the current in env (that is inside the goalkeeper) but instead it would be set right before the kick in front of the goalkeeper, at kick direction, at distance = settings.after_catch_distance
        # is ball reachable ? then catch it
        elif ( dist< self.settings.catchable_distance ) and ( (p.x >= self.pitch.x2) and (p.x <= self.pitch.area_lx) and (p.y <= self.pitch.area_uy) and (p.y >= self.pitch.area_dy) ) and ( cosinus_player_ball >=self.cos_catchable_angle ): # -catchable_angle >= goalkeeper-ball angle <= catchable_angle 
            if self.prev_action[0]==r2s.ACTION_KICK:   
                    return self._position_goalkeeper()       
            action= [r2s.ACTION_CATCH, 0.0, 0.0, 0.0]  
        else:  # otherwise position yourself in the right way
            return self._position_goalkeeper()

        return action

    def step(self, env, pitch, settings, team1, team2):
        self.normal_env = env
        self.pitch = pitch
        self.settings = settings
        self.cos_kickable_angle = math.cos(settings.kickable_angle)
        self.cos_catchable_angle = math.cos(settings.catchable_angle)

        self.teams = [team1, team2]
        self.normal_teams = [team1, team2]
        action=[r2s.ACTION_NOOP, 0.0, 0.0, 0.0]
        
        self.go_back_threshold_distance=self.pitch.x2/2.0
        self.go_back_baricenter_threshold_distance=self.pitch.x2/4.0
        self.run_forward_threshold_distance=self.pitch.x1*2/3.0
        
        self._transform_env_if_necessary()

        #if self.env.state == r2s.STATE_GOAL1:
        #    print("*GOAL ! "+str(self.env.tick))
        #if self.env.state == r2s.STATE_KICKOFF2:
        #    print("*KICKOFF2 "+str(self.env.tick))
        #if self.env.state == r2s.STATE_GOAL2:
        #    print("*GOAL received! "+str(self.env.tick))
        #if self.env.state == r2s.STATE_KICKOFF1:
        #    print("*KICKOFF1 "+str(self.env.tick) )

        if self.env.state==r2s.STATE_KICKOFF1 :
            x=-10.0
            y=0.0
            
            if self.shirt_number==0:
                x=(self.pitch.x2+ self.pitch.area_lx)/2.0
                y=0.0
            elif self.shirt_number==1:
                x=-self.settings.ball_radius-self.settings.player_radius-R2EPSILON
                y=0.0
            elif self.shirt_number==2:
                x=-self.settings.ball_radius
                y=self.settings.center_radius
            elif self.shirt_number==3:
                x=-pitch.x1/3.0
                y=-self.settings.center_radius
            else :
                x=-pitch.x1+pitch.x1/(self.shirt_number-1)
                y= self.settings.center_radius*( (self.shirt_number%3) -1)
            
            action=[r2s.ACTION_MOVE, x, y, 0.0]
                 
        elif self.env.state==r2s.STATE_KICKOFF2 :
            x=-10.0
            y=0.0
            
            if self.shirt_number==0:
                x=(self.pitch.x2+ self.pitch.area_lx)/2.0
                y=0.0
            elif self.shirt_number==1:
                x=-self.settings.center_radius-self.settings.player_radius-R2EPSILON
                y=0.0
            elif self.shirt_number==2:
                x=self.pitch.x2/2
                y=self.pitch.y2/2
            elif self.shirt_number==3:
                x=self.pitch.x2/2
                y=-self.pitch.y2/2
            else:
                x=-self.pitch.x1+self.pitch.x1/(self.shirt_number-1)
                y= self.settings.center_radius*( (self.shirt_number%3) -1)
        
            action=[r2s.ACTION_MOVE, x, y, 0.0]
            
        elif self.env.state==r2s.STATE_PLAY :
            if self.shirt_number==0:
                action= self._play_goalkeeper()
            else:
                action= self._play()

        elif self.env.state==r2s.STATE_GOALKICK1UP: # make the goalkeeper kick
            if self.shirt_number==0:
                action=[r2s.ACTION_MOVE, 
                        self.pitch.goal_kick_lx-(self.settings.kick_radius/10.0+self.settings.player_radius+self.settings.ball_radius),
                        self.pitch.goal_kick_uy,
                        0.0]

        elif self.env.state==r2s.STATE_GOALKICK1DOWN:
            if self.shirt_number==0:
                action=[r2s.ACTION_MOVE,
                        self.pitch.goal_kick_lx-(self.settings.kick_radius/10+self.settings.player_radius+self.settings.ball_radius),
                        self.pitch.goal_kick_dy,
                        0.0]

        elif self.env.state==r2s.STATE_CORNER1UP: # the closest to the ball go there
            if self._my_player_closest_to_ball()==self.shirt_number:
                displace=self.settings.kick_radius/10+self.settings.player_radius+self.settings.ball_radius
                action=[r2s.ACTION_MOVE,
                        self.pitch.x1,
                        self.pitch.y1+displace,
                        0.0]
            
        elif self.env.state==r2s.STATE_CORNER1DOWN:
            if self._my_player_closest_to_ball()==self.shirt_number:
                displace=self.settings.kick_radius/10+self.settings.player_radius+self.settings.ball_radius
                action=[r2s.ACTION_MOVE,
                        self.pitch.x1,
                        self.pitch.y2-displace,
                        0.0]
            
        elif self.env.state==r2s.STATE_THROWIN1:  # the closest to the ball go there
            if self._my_player_closest_to_ball()==self.shirt_number:
                displace=self.settings.kick_radius/10+self.settings.player_radius+self.settings.ball_radius
                if self.env.ball_y<0.0:
                    displace*=-1
                action=[r2s.ACTION_MOVE,
                        self.env.ball_x,
                        self.env.ball_y+displace,
                        0.0]

        self.prev_state=self.env.state
        self.prev_action=self._transform_action_if_necessary(action)
        return self.prev_action

######################## The functions below are only for testing purposes ##################

def _test_build_simulator():
    my_team1=[HumblePlayer(0,i) for i in range(4)]
    my_team2=[HumblePlayer(1,i) for i in range(4)]
    return r2s.build_simpleplayer_simulator(my_team1, 0, my_team2, 0, "The Snakes", "Pacific United", 42)

def _test_gui():
    import robosoc2dgui.gui as r2gui
    r2gui.play_whole_game(_test_build_simulator(), waiting_time=0.01)

#to be used only on jupyter notebook, with "%matplotlib notebook"
def _test_plot_notebook():
    import robosoc2dplot.plot as r2plot
    r2plot.play_steps_in_notebook(_test_build_simulator(), waiting_time=0.01)

#to be used only on jupyter notebook, with "%matplotlib inline"
def _test_plot_inline():
    import robosoc2dplot.plot as r2plot
    r2plot.play_steps_in_notebook_inline(_test_build_simulator(), waiting_time=0.01)

#to be used only on jupyter notebook
def _test_plotty():
    import robosoc2dplotty.plotty as r2plotty
    r2plotty.play_steps_in_notebook(_test_build_simulator(), waiting_time=0.01)

def _test_console():
    sim_handle = _test_build_simulator()
    while r2s.simulator_step_if_playing(sim_handle):
        print(r2s.simulator_get_state_string(sim_handle))

if __name__ == "__main__":
    #_test_gui()
    _test_console()
