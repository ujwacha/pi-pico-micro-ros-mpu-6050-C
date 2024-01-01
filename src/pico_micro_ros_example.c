#include "lib.c"


#define PUBMSG ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist)
#define SUBMSG ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist)


const uint LED_PIN = 25;

int current_led_value = 1;

rcl_publisher_t publisher; 
rcl_subscription_t subescriber; 


int16_t acceleration[3], gyro[3], temp;



Node current_node;
geometry_msgs__msg__Twist msg;
geometry_msgs__msg__Twist from_sub_msg;


void flip_led() {
  if (current_led_value) {
    current_led_value = 0;
  } else {
    current_led_value = 1;
  }
}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{


  
  mpu6050_read_raw(acceleration, gyro, &temp);

  msg.angular.x = acceleration[0];
  msg.angular.y = acceleration[1];
  msg.angular.z = acceleration[2];


  msg.linear.x = gyro[0];
  msg.linear.y = gyro[1];
  msg.linear.z = gyro[2];


  rcl_ret_t ret = rcl_publish(&current_node.publisher, &msg, NULL);
}


void subescription_callback(const void * mess) {

  const geometry_msgs__msg__Twist* message = (const geometry_msgs__msg__Twist*) mess;

  flip_led();

   gpio_put(LED_PIN, current_led_value);
  
  msg = *message;

}



int main() {

  rcl_ret_t ret;



  stdio_init_all();
  
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  
  

  // get the node without the boilerplate bullshit

  get_node(&current_node,
	   "node",
	   "pico",
	   
	   
	   "datacome",
	   PUBMSG,

	   
	   
	   "givdata",
	   SUBMSG,
	   
	   1000,
	   120);
  

  // initiate the timer

  rclc_timer_init_default(&current_node.timer,
			  &current_node.supprot,
			  RCL_MS_TO_NS(100),
			  timer_callback);


  // initiate the executor
  rclc_executor_init(&current_node.executor,
		     &current_node.supprot.context,
		     2,
		     &current_node.allocator);

  // add timer in the executor to publish stuff
  rclc_executor_add_timer(&current_node.executor,
			  &current_node.timer);

  // add the subescriber in the executor

 
  rclc_executor_add_subscription(&current_node.executor,
				 &current_node.subscriber,
				 &from_sub_msg,
				 subescription_callback,
				 ON_NEW_DATA);
  


  
  gpio_put(LED_PIN, 1);



  // set up mpu
  
  i2c_init(i2c0, 400 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
  // Make the I2C pins available to picotool
  //  bi_decl(bi_2pins_with_func(4, 5, GPIO_FUNC_I2C));

  
  mpu6050_reset();



  msg.angular.x = 1;
  msg.angular.y = 1;
  msg.angular.z = 1;

  msg.linear.x = 1;
  msg.linear.y = 1;
  msg.linear.z = 1;



  while (true) {
      rclc_executor_spin_some(&current_node.executor, RCL_MS_TO_NS(100));
    }

  return 0;
}
