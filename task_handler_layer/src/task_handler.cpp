#include <task_handler.h>

taskHandler::taskHandler (double _timeout) {
    topic_map_["upward"] = "/anahita/z_coordinate";
    topic_map_["sideward"] = "/anahita/y_coordinate";
    topic_map_["forward"] = "/anahita/x_coordinate";
    topic_map_["angle"] = "/mavros/imu/yaw";
    topic_map_["pitch"] = "/mavros/imu/pitch";
    topic_map_["roll"] = "/mavros/imu/roll";

    task_map_["red_buoy"] = false;
    task_map_["green_buoy"] = false;
    task_map_["yellow_buoy"] = false;
    task_map_["buoy-gate"] = false;
    task_map_["gate_front"] = false;
    task_map_["gate_bottom"] = false;
    task_map_["red_torpedo"] = false;
    task_map_["green_torpedo"] = false;
    task_map_["marker_dropper_bottom"] = false;
    task_map_["marker_dropper_front"] = false;
    task_map_["octagon"] = false;
    task_map_["line"] = false;

    time_out_ =  _timeout;

    vision_sub_ = nh_.subscribe("/detected", 1, &taskHandler::visionCB, this);
}

taskHandler::~taskHandler () {}

void taskHandler::callBack (const std_msgs::Float32Ptr &_msg) {
    // data_mutex.lock();
    data_ = _msg->data;
    // data_mutex.unlock();

    // mtx.lock();
    dataReceived = true;
    // mtx.unlock();
}

bool taskHandler::isAchieved (double _target, double _band, std::string _topic) {

    double beginning = ros::Time::now().toSec();

    if (is_subscribed_) {
        sub_.shutdown();
    }
    sub_ = nh_.subscribe(topic_map_[_topic], 1, &taskHandler::callBack, this);
    is_subscribed_ = true;

    int count = 0;
    double then = ros::Time::now().toSec();

    if (_topic == "angle") {

        double temp = 0;

        bool use_reference_yaw = false;
        nh_.getParam("/use_reference_yaw", use_reference_yaw);

        bool use_local_yaw = false;
        nh_.getParam("/use_local_yaw", use_local_yaw);

        if (use_reference_yaw) {
            double reference_angle = 0;
            nh_.getParam("/reference_yaw", reference_angle);
            temp = reference_angle + _target;
        }
        else if (use_local_yaw) {
            double local_angle = 0;
            nh_.getParam("/local_yaw", local_angle);
            temp = local_angle + _target;
        }
        else {
            while (ros::ok()) {
                // mtx.lock();
                bool dataReceived_ = dataReceived;
                // mtx.unlock();
                if (dataReceived_) { break; }
            }
            // data_mutex.lock();
            temp = data_ + _target;
            // data_mutex.unlock();
        }

        if (temp > 180) {
            temp = temp - 360;
        }
        else if (temp < -180) {
            temp = temp + 360;
        }
        _target = temp;
    }

    ros::Rate loop_rate(100);

    while (ros::ok()) {
        // data_mutex.lock();
        double data = data_;
        // data_mutex.unlock();

        if (std::abs(data - _target) <= _band) {
            if (!count) {
                then = ros::Time::now().toSec();
            }
            count++;
        }        

        double now = ros::Time::now().toSec();
        double diff = now - then;
        double total_time = now - beginning;

        if (total_time >= time_out_) {
            ROS_INFO("Failed");
            return false;
        }

        if (diff >= 1.0) {
            if (count >= 100) {
                return true;
            }
            else {
                count = 0;
            }
        }
    	loop_rate.sleep();
    }
    return true;
}

bool taskHandler::isDetected (std::string _task, double _timeout) {
    vision_time_out_ = _timeout;
    double then = ros::Time::now().toSec();
    double now;
    double diff;
    while (ros::ok()) {
        // vision_mutex.lock();
        bool temp = task_map_[_task];
        // vision_mutex.unlock();
        if (temp) {
            break;
        }
        now = ros::Time::now().toSec();
        diff = now - then;
        if (diff > vision_time_out_) {
            return false;
        }
    }

    return true;
}

void taskHandler::visionCB (const std_msgs::BoolPtr& _msg) {
    std::string current_task;
    nh_.getParam("/current_task", current_task);

    if (_msg->data) {
        // vision_mutex.lock();
        if (current_task == "green_buoy") {
            task_map_["green_buoy"] = true;
        }
        else if (current_task == "yellow_buoy") {
            task_map_["yellow_buoy"] = true;
        }
        else if (current_task == "red_buoy") {
            task_map_["red_buoy"] = true;
        }
        else if (current_task == "gate_front") {
            task_map_["gate_front"] = true;
        }
        else if (current_task == "gate_bottom") {
            task_map_["gate_bottom"] = true;
        }
        else if (current_task == "red_torpedo") {
            task_map_["red_torpedo"] = true;
        }
        else if (current_task == "green_torpedo") {
            task_map_["green_torpedo"] = true;
        }
        else if (current_task == "octagon") {
            task_map_["octagon"] = true;
        }
        else if (current_task == "marker_dropper_front") {
            task_map_["marker_dropper_front"] = true;
        }
        else if (current_task == "marker_dropper_bottom") {
            task_map_["marker_dropper_bottom"] = true;
        }
        else if (current_task == "line") {
            task_map_["line"] = true;
        }
        // vision_mutex.unlock();
    } 
}

void taskHandler::setTimeout (double _time) {
    time_out_ = _time;
}

void taskHandler::setVisionTimeout (double _time) {
    vision_time_out_ = _time;
}
