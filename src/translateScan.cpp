#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <laser_geometry/laser_geometry.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_sequencer.h>

/**
 * Scan2PointTranslator
 * Translates the points acquired from LaserScan to PointCloud2
 */
class Scan2PointTranslator {
    public:
        Scan2PointTranslator();
        void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan);
    private:
        ros::NodeHandle nh;
        tf::TransformListener listener;

        // laser
        laser_geometry::LaserProjection lp;
        ros::Publisher point_cloud_publisher;
        message_filters::Subscriber<sensor_msgs::LaserScan> scan_sub;
        // ros::Subscriber scan_sub;
        // sensor_msgs::PointCloud2 filterCloud(const sensor_msgs::PointCloud2& scanPC);
};

Scan2PointTranslator::Scan2PointTranslator() {
    message_filters::Subscriber<sensor_msgs::LaserScan> scan_sub(nh, "/lidar_scan", 10);
    message_filters::TimeSequencer<sensor_msgs::LaserScan> seq(scan_sub, ros::Duration(0.1), ros::Duration(0.01), 10);
    // seq.registerCallback(boost::bind(&Scan2PointTranslator::scanCallback, this, _1));
    seq.registerCallback(&Scan2PointTranslator::scanCallback, this);
    // scan_sub = nh.subscribe<sensor_msgs::LaserScan> ("/lidar_scan", 10, &Scan2PointTranslator::scanCallback, this);
    point_cloud_publisher = nh.advertise<sensor_msgs::PointCloud2>("/sync_scan_cloud_filtered", 2);
}

void Scan2PointTranslator::scanCallback( const sensor_msgs::LaserScan::ConstPtr &scan ) {
  ROS_INFO_STREAM("HERE!");
  try {
      if(!listener.waitForTransform(
          scan->header.frame_id,
          "/base_link",
          scan->header.stamp + ros::Duration().fromSec(scan->ranges.size()*scan->time_increment),
          ros::Duration(2.0))) {
          return;
      }
  } catch(tf::TransformException &ex) {
          ROS_ERROR("%s", ex.what());
          ros::Duration(1.0).sleep();
          return;
  }
   
  sensor_msgs::PointCloud2 cloud;
  lp.transformLaserScanToPointCloud( "/base_link", *scan, cloud, listener);
  point_cloud_publisher.publish(cloud);
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "translateScan");

    Scan2PointTranslator s2pt;

    ros::spin();

    return 0;
}