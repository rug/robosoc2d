# (c) 2021 Ruggero Rossi
# Load a Robosoc2d game state log
# supported version: 1.0.0

def load_state_log(file_name):
    history = None
    with open(file_name, 'r') as f:

        game={}
        version=f.readline()
        if len(version)==0 :
            return None
        game['ver']=version

        game['team1_name']=f.readline().strip()
        if len(game['team1_name']) == 0:
            game['team1_name'] = "Team A"

        game['team2_name']=f.readline().strip()
        if len(game['team2_name']) == 0:
            game['team2_name'] = "Team B"

        players=f.readline()
        if len(players)==0 :
            return None
        
        players= players.split(',')
        if len(players)<2:
            return None
        
        game['n_players']=[]
        if players[0].isdigit():
            game['n_players'].append(int(players[0]))
        else:
            return None
 
        players[1]=players[1].strip('\n')
        if players[1].isdigit():
            game['n_players'].append(int(players[1]))
        else:
            return None
        
        settings=f.readline()
        if len(settings)==0 :
            return None
        settings=settings.split(',')
        if len(settings) < 34 :
            return None

        sett={}
        sett['ticks_per_time']=int(settings[0])
        sett['pitch_length']=float(settings[1])
        sett['pitch_width']=float(settings[2])
        sett['goal_width']=float(settings[3])
        sett['center_radius']=float(settings[4])
        sett['pole_radius']=float(settings[5])
        sett['ball_radius']=float(settings[6])
        sett['player_radius']=float(settings[7])
        sett['catch_radius']=float(settings[8])
        sett['catch_holding_ticks']=int(settings[9])
        sett['kick_radius']=float(settings[10])
        sett['kickable_distance']=float(settings[11])
        sett['catchable_distance']=float(settings[12])
        sett['kickable_angle']=float(settings[13])
        sett['kickable_direction_angle']=float(settings[14])
        sett['catchable_angle']=float(settings[15])
        sett['net_length']=float(settings[16])
        sett['catchable_area_length']=float(settings[17])
        sett['catchable_area_width']=float(settings[18])
        sett['corner_min_distance']=float(settings[19])
        sett['throwin_min_distance']=float(settings[20])
        sett['out_pitch_limit']=float(settings[21])
        sett['max_dash_power']=float(settings[22])
        sett['max_kick_power']=float(settings[23])
        sett['player_velocity_decay']=float(settings[24])
        sett['ball_velocity_decay']=float(settings[25])
        sett['max_player_speed']=float(settings[26])
        sett['max_ball_speed']=float(settings[27])
        sett['catch_probability']=float(settings[28])
        sett['player_random_noise']=float(settings[29])
        sett['player_direction_noise']=float(settings[30])
        sett['player_velocity_direction_mix']=float(settings[31])
        sett['ball_inside_player_velocity_displace']=float(settings[32])
        sett['after_catch_distance']=float(settings[33].strip('\n'))

        game['sett']=sett

        ticks=[]

        min_line_len=offset=8+game['n_players'][0]*5+game['n_players'][1]*5+4
        default_empty=[0]*min_line_len
        prev_tick=default_empty

        for tick in f:
            tick=tick.split(',')

            if len(tick) < min_line_len:
                print("* error: missing data at tick: "+str(len(ticks)))
                tick=prev_tick
            
            t={}
            t['score1']=int(tick[1])
            t['score2']=int(tick[2])
            t['state']=int(tick[3])
            t['ball_x']=float(tick[4])
            t['ball_y']=float(tick[5])
            t['ball_velocity_x']=float(tick[6])
            t['ball_velocity_y']=float(tick[7])
            t['teams']=[[],[]]
            
            offset=game['n_players'][0]*5

            for which_team in range(2):
                for i in range(game['n_players'][which_team]):
                    p={}
                    p['x']=float(tick[i*5+8+offset*which_team])
                    p['y']=float(tick[i*5+9+offset*which_team])
                    p['velocity_x']=float(tick[i*5+10+offset*which_team])
                    p['velocity_y']=float(tick[i*5+11+offset*which_team])
                    p['direction']=float(tick[i*5+12+offset*which_team])
                    t['teams'][which_team].append(p)

            offset=(game['n_players'][0]+game['n_players'][1])*5
            t['last_touched_team2']=bool(int(tick[8+offset]))
            t['starting_team_max_range']=float(tick[9+offset]) 
            t['ball_catched']=int(tick[10+offset])
            t['ball_catched_team2']=bool(int(tick[11+offset].strip('\n')))

            ticks.append(t)
            prev_tick=tick
            
        game['ticks']=ticks
        history=game

    return history