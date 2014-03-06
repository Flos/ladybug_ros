/*
 * colorSepToImage.cpp
 *
 *  Created on: 15.02.2014
 *      Author: fnolden
 */

#include "image_publisher.h"

image_publisher::image_publisher(std::string subscribe_topic) {
	//Topic you want to subscribe
	subscribe_topic_ = subscribe_topic;

	ROS_INFO_STREAM("Creating processing node for topic" << subscribe_topic.c_str());
	sub_ = n_.subscribe(subscribe_topic.c_str(), 1, &image_publisher::callback, this);
	it_ = NULL;
}

image_publisher::~image_publisher() {
	ROS_INFO_STREAM("closing node for topic" << subscribe_topic_);
	delete it_;
}

void
image_publisher::callback(const ladybug::image &input)
{
	if(it_ == NULL){
		// Create image transport
		it_ = new image_transport::ImageTransport(n_);
		pub_ = it_->advertise(getTopicNameRawImage(input.camera_number), 1);

		// Create Transform
		tf::Quaternion quat;
		quat.setRPY(input.rotationX, input.rotationY, input.rotationZ);
		transform.setOrigin( tf::Vector3(input.translationX, input.translationY, input.translationZ) );
		transform.setRotation(quat);

		// Create CameraInfo
		cam_info_msg.header = input.header;
		cam_info_msg.width = input.width;
		cam_info_msg.height = input.height;
		cam_info_msg.distortion_model = sensor_msgs::distortion_models::PLUMB_BOB;
		//cam_info_msg.D[0] =
		cam_info_msg.K[0] = input.focalX;
		cam_info_msg.K[2] = input.centerX;
		cam_info_msg.K[4] = input.focalY;
		cam_info_msg.K[5] = input.centerY;
		cam_info_msg.K[8] = 1;
	    pub_info_ = n_.advertise<sensor_msgs::CameraInfo>(getTopicName(input.camera_number)+"/camera_info", 1);
	}
	br.sendTransform(tf::StampedTransform(transform, input.header.stamp, "ladybug_link", input.header.frame_id));
	cam_info_msg.header.stamp = input.header.stamp;
	pub_info_.publish(cam_info_msg);
	pub_.publish(createImgPtr(&input));
}

