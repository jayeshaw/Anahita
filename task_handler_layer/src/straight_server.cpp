#include <straight_server.h>

moveStraight::moveStraight(int pwm_) : anglePIDClient("turnPID") { 
    nh_.setParam("/pwm_surge", pwm_); 
    // sub_ = nh_.subscribe("/mavros/imu/yaw", 1, &moveStraight::angleCB, this);
}

moveStraight::~moveStraight() {
}

void moveStraight::setActive(bool status, std::string type) {
    if (status == true) {
        if (type == "reference") { nh_.setParam("/use_reference_yaw", true); }
        else if (type == "local") { nh_.setParam("/use_local_yaw", true); }
        ros::Duration(1).sleep();
        spin_thread = new boost::thread(boost::bind(&moveStraight::spinThread, this));
    }
    else {
        nh_.setParam("/use_reference_yaw", false);
        nh_.setParam("/use_local_yaw", false);
        anglePIDClient.cancelGoal();
        spin_thread->join();
        nh_.setParam("/kill_signal", true);
    }
    // else {
    //     if (status == true) {
    //         spin_thread_ = new boost::thread(boost::bind(&moveStraight::spinThread_, this));
    //     }
    //     else {
    //         anglePIDClient.cancelGoal();
    //         spin_thread_->join();
    //         nh_.setParam("/kill_signal", true);
    //     }
    // }
}   

void moveStraight::spinThread() {

    ROS_INFO("Waiting for turnPID server to start.");
    anglePIDClient.waitForServer();
    ROS_INFO("turnPID server started, sending goal.");
    angle_PID_goal.target_angle = 0;
    anglePIDClient.sendGoal(angle_PID_goal);
}

void moveStraight::setThrust(int _pwm) {
    ros::Duration(1).sleep();
    nh_.setParam("/pwm_surge", _pwm);
}

// void moveStraight::spinThread_() {
//     ROS_INFO("Waiting for turnPID server to start.");
//     anglePIDClient.waitForServer();
//     ROS_INFO("turnPID server started, sending goal.");
//     while (ros::ok()) {
//         // mtx.lock();
//         bool temp = angleReceived;
//         // mtx.unlock();
//         if (temp) {
//             break;
//         }
//     }
//     double reference_angle = 0;
//     nh_.getParam("/reference_yaw", reference_angle);
//     // mtx.lock();
//     angle_PID_goal.target_angle = angle_ - reference_angle;
//     // mtx.unlock();
//     anglePIDClient.sendGoal(angle_PID_goal);
// }

// void moveStraight::angleCB(const std_msgs::Float32ConstPtr& _msg) {
//     // mtx.lock();
//     angle_ = _msg->data;
//     angleReceived = true;
//     // mtx.unlock();
// }
