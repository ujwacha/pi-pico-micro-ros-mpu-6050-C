#include <stdio.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
//#include <std_msgs/msg/string.h>
#include <rmw_microros/rmw_microros.h>


#include "hardware/gpio.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico_uart_transports.h"
#include "rcl/allocator.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rclc/executor_handle.h"
#include "rclc/init.h"
#include "rclc/node.h"
#include "rclc/publisher.h"
#include "rclc/subscription.h"
#include "rclc/types.h"
#include "rmw_microros/ping.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "std_msgs/msg/detail/int32__struct.h"
#include "std_msgs/msg/detail/string__functions.h"
#include "std_msgs/msg/detail/string__struct.h"
#include "geometry_msgs/msg/twist.h"
#include "geometry_msgs/msg/detail/twist__struct.h"
#include "mpu6050.c"


typedef struct {
  rcl_publisher_t publisher;
  rcl_subscription_t subscriber;
  rcl_node_t node;
  rcl_timer_t timer;
  rcl_allocator_t allocator;
  rclc_support_t supprot;
  rclc_executor_t executor;
} Node;



void get_node(
	      Node* node_class,
	      char*nodename,
	      char* namespace,

	      char* publisher_name,
	      const rosidl_message_type_support_t* publisher_message_type,
	      
	      char* subscriber_name,
	      const rosidl_message_type_support_t* subscriber_message_type,


	      int timeout_ms,
	      uint8_t attempts
)
{


  rcl_ret_t ret;
  

  // do some stuff so that we can talk to the serial
  rmw_uros_set_custom_transport(
				true,
				NULL,
				pico_serial_transport_open,
				pico_serial_transport_close,
				pico_serial_transport_write,
				pico_serial_transport_read
				);
  


  
  node_class->allocator = rcl_get_default_allocator();

  // wait for some time to get access from the pi

  ret = rmw_uros_ping_agent(timeout_ms, attempts);

  // check for error
  // wait for 2 minutes and it it fails panic
  if (ret != RCL_RET_OK) {
    panic("Couldn't fine ROS computer connected to me. ERROR: %i", ret);
  }

  // init support and allocator

  rclc_support_init(&node_class->supprot,
		    0,
		    NULL,
		    &node_class->allocator);
  

  rclc_node_init_default(&node_class->node,
			 nodename,
			 namespace,
			 &node_class->supprot);



  // error handling publisher
  if (publisher_name != NULL) {
    ret = rclc_publisher_init_default(&node_class->publisher,
				      &node_class->node,
				      publisher_message_type,
				      publisher_name);

    if (ret != RCL_RET_OK) {

      ret = rcl_publisher_fini(&node_class->publisher,
			       &node_class->node);
      panic("Couldn't make publisher");
    }
  }


  //error handling subscriber
  if (subscriber_name != NULL) {

    ret = rclc_subscription_init_default(&node_class->subscriber,
					 &node_class->node,
					 subscriber_message_type,
					 subscriber_name);


    if (ret != RCL_RET_OK) {
      // get rid of both publisher and subescriber
      
      ret = rcl_publisher_fini(&node_class->publisher,
			       &node_class->node);
      ret = rcl_subscription_fini(&node_class->subscriber, &node_class->node);
      panic("Failed Making subscriber");

    }
  }
}



