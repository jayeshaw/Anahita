#include <ros/ros.h>
#include <std_msgs/Float32.h>

double depth_data = 0;

void pressureCB (const std_msgs::Float32 _msg) {
    depth_data = _msg.data;
}

int main (int argc, char** argv) {
    ros::init(argc, argv, "pressure_publisher");
    ros::NodeHandle nh;

    ros::Subscriber pressure_sub = nh.subscribe<std_msgs::Float32>("/pressure_sensor/depth", 1000, &pressureCB);
    ros::Publisher depth_pub = nh.advertise<std_msgs::Float32>("/anahita/z_coordinate", 1000);

    bool enable_pressure = false;
    std_msgs::Float32 depth;

    ros::Rate loop_rate(50);

    while (ros::ok()) {
        nh.getParam("/enable_pressure", enable_pressure);
        if (enable_pressure) {
            depth.data = depth_data;
            depth_pub.publish(depth);
        }
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}