#!/usr/bin/env python

import rospy
from visualization_msgs.msg import Marker
from visualization_msgs.msg import MarkerArray
from std_msgs.msg import String
from rws2016_msgs.msg import GameMove
import random
import tf
import math
import os

score = [0,0,0]
pub = rospy.Publisher('game_move', GameMove, queue_size=0)
pub_referee = rospy.Publisher("referee_markers", MarkerArray, queue_size=10)
pub_score = rospy.Publisher("score_markers", MarkerArray, queue_size=10)
pub_rip = rospy.Publisher("kill_markers", MarkerArray, queue_size=10)
pub_killer = rospy.Publisher("victim", String, queue_size=10)

rate = 0
game_duration = rospy.get_param('/game_duration')
positive_score = rospy.get_param('/positive_score')
negative_score = rospy.get_param('/negative_score')
killed = [];


def timerCallback(event):

    global pub

    a = GameMove()
    
    a.header.frame_id = "";
    #a.header.stamp = rospy.get_time()
    a.cheetah = random.random()/10
    a.dog = random.random()/10
    a.cat = random.random()/10
    a.turtle = random.random()/10
    if not rospy.is_shutdown():
        pub.publish(a)

def gameEndCallback(event):

    rospy.loginfo("Game finished")
    global pub_score

    ma = MarkerArray()

    m1 = Marker()
    m1.header.frame_id = "/map"
    m1.type = m1.TEXT_VIEW_FACING
    m1.action = m1.ADD
    m1.id = 777;
    m1.scale.x = 0.2
    m1.scale.y = 0.2
    m1.scale.z = 0.9
    m1.color.a = 1.0
    m1.color.r = 0.1
    m1.color.g = 0.1
    m1.color.b = 0.1
    m1.pose.position.x = .5
    m1.pose.position.y = 4.5
    m1.pose.position.z = 0 

    if score[0] > score[1] and score[0] > score[2]:
        m1.text = "Team A wins the game"
    elif score[1] > score[0] and score[1] > score[2]:
        m1.text = "Team B wins the game"
    elif score[2] > score[0] and score[2] > score[1]:
        m1.text = "Team C wins the game"

    ma.markers.append(m1)
    pub_score.publish(ma)
    rate.sleep()

    rospy.signal_shutdown("Game finished")

def talker():
    rospy.init_node('referee', anonymous=True)
    listener = tf.TransformListener()
    global rate 
    rate = rospy.Rate(2) # 10hz
    rate.sleep()

    hunting_distance = rospy.get_param('/hunting_distance')
    teamA = rospy.get_param('/teamRed')
    teamB = rospy.get_param('/teamGreen')
    teamC = rospy.get_param('/teamBlue')

    rospy.Timer(rospy.Duration(0.01), timerCallback, oneshot=False)
    rospy.Timer(rospy.Duration(game_duration), gameEndCallback, oneshot=True)
    game_start = rospy.get_time()

    while not rospy.is_shutdown():


        print killed
        tic = rospy.Time.now()
        for i in killed:
            d = tic-i[1]
            if d.to_sec() > 10:
                rospy.logwarn("Ressuscitating %s", i[0])
                killed.remove(i)



        print killed
        rospy.loginfo("Checking ...")

        to_be_killed_A = [];
        to_be_killed_B = [];
        to_be_killed_C = [];
        #Team A hunting team B
        for hunter in teamA:
            
            #check if hunter is killed
            result = [item for item in killed if item[0] == hunter]
            if not result:
                for prey in teamB:
                    try:
                        (trans,rot) = listener.lookupTransform(hunter, prey, rospy.Time(0))
                        distance = math.sqrt(trans[0]*trans[0] + trans[1]*trans[1])
                        #rospy.loginfo("D from %s to %s is %f", hunter, prey, distance)
                        if distance < hunting_distance:
                            to_be_killed_A.append(prey);
                    except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
                        t=1
                        #rospy.loginfo("Referee: tf error")

        #Team A hunting team B
        for hunter in teamB:
            #check if hunter is killed
            result = [item for item in killed if item[0] == hunter]
            if not result:
                for prey in teamC:
                    try:
                        (trans,rot) = listener.lookupTransform(hunter, prey, rospy.Time(0))
                        distance = math.sqrt(trans[0]*trans[0] + trans[1]*trans[1])
                        #rospy.loginfo("D from %s to %s is %f", hunter, prey, distance)
                        if distance < hunting_distance:
                            to_be_killed_B.append(prey);
                    except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
                        t=1
                        #rospy.loginfo("Referee: tf error")

        #Team C hunting team A
        for hunter in teamC:
            #check if hunter is killed
            result = [item for item in killed if item[0] == hunter]
            if not result:
                for prey in teamA:
                    try:
                        (trans,rot) = listener.lookupTransform(hunter, prey, rospy.Time(0))
                        distance = math.sqrt(trans[0]*trans[0] + trans[1]*trans[1])
                        #rospy.loginfo("D from %s to %s is %f", hunter, prey, distance)
                        if distance < hunting_distance:
                            to_be_killed_C.append(prey);
                    except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
                        t=1
                        #rospy.loginfo("Referee: tf error")


        ma_killed = MarkerArray()



        kill_time = rospy.Time.now()
        for tbk in to_be_killed_A:
            #rospy.logwarn("%s is to be killed by Team A", tbk)

            #Check if tbk is in the list of killed players
            found = False
            for item in killed:
                if item[0] == tbk:
                    found = True
            
            if found == False:
                killed.append((tbk,kill_time))
                rospy.logwarn("Team A hunted %s", tbk)
                score[0] = score[0] + positive_score
                score[1] = score[1] + negative_score
                s = String()
                s.data = tbk
                pub_killer.publish(s)
                mk1 = Marker()
                mk1.header.frame_id = tbk
                mk1.type = mk1.TEXT_VIEW_FACING
                mk1.action = mk1.ADD
                mk1.id = 0;
                mk1.ns = tbk
                mk1.scale.z = 0.2
                mk1.color.a = 1.0
                mk1.text = "RIP " + tbk
                mk1.lifetime = rospy.Duration.from_sec(5)
                ma_killed.markers.append(mk1)

            else:
                t=1
                #rospy.logwarn("%s was already hunted", tbk)


        for tbk in to_be_killed_B:
            #rospy.logwarn("%s is to be killed by Team B", tbk)

            #Check if tbk is in the list of killed players
            found = False
            for item in killed:
                if item[0] == tbk:
                    found = True
            
            if found == False:
                killed.append((tbk,kill_time))
                rospy.logwarn("Team B hunted %s", tbk)
                score[1] = score[1] + positive_score
                score[2] = score[2] + negative_score
                s = String()
                s.data = tbk
                pub_killer.publish(s)
                mk1 = Marker()
                mk1.header.frame_id = tbk
                mk1.type = mk1.TEXT_VIEW_FACING
                mk1.action = mk1.ADD
                mk1.id = 0;
                mk1.ns = tbk
                mk1.scale.z = 0.2
                mk1.color.a = 1.0
                mk1.text = "RIP " + tbk
                mk1.lifetime = rospy.Duration.from_sec(5)
                ma_killed.markers.append(mk1)

            else:
                t=1
                #rospy.logwarn("%s was already hunted", tbk)

            
        for tbk in to_be_killed_C:
            #rospy.logwarn("%s is to be killed by Team C", tbk)

            #Check if tbk is in the list of killed players
            found = False
            for item in killed:
                if item[0] == tbk:
                    found = True
            
            if found == False:
                killed.append((tbk,kill_time))
                rospy.logwarn("Team C hunted %s", tbk)
                score[2] = score[2] + positive_score
                score[0] = score[0] + negative_score
                s = String()
                s.data = tbk
                pub_killer.publish(s)
                mk1 = Marker()
                mk1.header.frame_id = tbk
                mk1.type = mk1.TEXT_VIEW_FACING
                mk1.action = mk1.ADD
                mk1.id = 0;
                mk1.ns = tbk
                mk1.scale.z = 0.2
                mk1.color.a = 1.0
                mk1.text = "RIP " + tbk
                mk1.lifetime = rospy.Duration.from_sec(5)
                ma_killed.markers.append(mk1)

            else:
                t=1
                #rospy.logwarn("%s was already hunted", tbk)
        
        if ma_killed.markers:
            pub_rip.publish(ma_killed)
        #print killed
                        

        #rospy.logwarn("Team A: %s hunted %s", hunter, prey)
        ##os.system('rosnode kill ' + prey)
        #score[0] = score[0] + positive_score
        #score[1] = score[1] + negative_score
        #s = String()
        #s.data = prey
        #pub_killer.publish(s)


        ma = MarkerArray()

        m1 = Marker()
        m1.header.frame_id = "/map"
        m1.type = m1.TEXT_VIEW_FACING
        m1.action = m1.ADD
        m1.id = 0;
        m1.scale.x = 0.2
        m1.scale.y = 0.2
        m1.scale.z = 0.6
        m1.color.a = 1.0
        m1.color.r = 1.0
        m1.color.g = 0.0
        m1.color.b = 0.0
        m1.text = "A=" + str(score[0])
        m1.pose.position.x = -5.0
        m1.pose.position.y = 5.2
        m1.pose.position.z = 0 
        ma.markers.append(m1)

        m2 = Marker()
        m2.header.frame_id = "/map"
        m2.type = m2.TEXT_VIEW_FACING
        m2.action = m2.ADD
        m2.id = 1;
        m2.scale.x = 0.2
        m2.scale.y = 0.2
        m2.scale.z = 0.6
        m2.color.a = 1.0
        m2.color.r = 0.0
        m2.color.g = 1.0
        m2.color.b = 0.0
        m2.text = "B=" + str(score[1])
        m2.pose.position.x = -3.0
        m2.pose.position.y = 5.2
        m2.pose.position.z = 0 
        ma.markers.append(m2)

        m3 = Marker()
        m3.header.frame_id = "/map"
        m3.type = m3.TEXT_VIEW_FACING
        m3.action = m3.ADD
        m3.id = 2;
        m3.scale.x = 0.2
        m3.scale.y = 0.2
        m3.scale.z = 0.6
        m3.color.a = 1.0
        m3.color.r = 0.0
        m3.color.g = 0.0
        m3.color.b = 1.0
        m3.text = "C=" + str(score[2])
        m3.pose.position.x = -1.0
        m3.pose.position.y = 5.2
        m3.pose.position.z = 0 
        ma.markers.append(m3)

        m4 = Marker()
        m4.header.frame_id = "/map"
        m4.type = m4.TEXT_VIEW_FACING
        m4.action = m4.ADD
        m4.id = 3;
        m4.scale.x = 0.2
        m4.scale.y = 0.2
        m4.scale.z = 0.6
        m4.color.a = 1.0
        m4.color.r = 0.0
        m4.color.g = 0.0
        m4.color.b = 1.0
        #,c rospy.Time(

        time_now = rospy.get_time()

        m4.text = "Time " + str(format(time_now-game_start, '.2f')) + " of " + str(game_duration)
        m4.pose.position.x = 3.5
        m4.pose.position.y = 5.2
        m4.pose.position.z = 0 
        ma.markers.append(m4)

        pub_score.publish(ma)
        rate.sleep()

if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass
