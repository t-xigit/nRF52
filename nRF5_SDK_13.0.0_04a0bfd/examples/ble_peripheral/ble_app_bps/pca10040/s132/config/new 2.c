#define APP_MQTT_SUBSCRIPTION_TOPIC         					target_to_store_nello_topic_subscribe_test                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_SUBSCRIPTION_TOPIC_STRAIGHT_DOOR_OPEN         	target_to_store_nello_topic_subscribe__straight_door_open                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_SUBSCRIPTION_TOPIC_GEOFENCE         			target_to_store_nello_topic_subscribe_geofence                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_SUBSCRIPTION_TOPIC_TIME_WINDOW         		target_to_store_nello_topic_subscribe_time_window                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_SUBSCRIPTION_TOPIC_BE_TO_NELLO_LEARNING        target_to_store_nello_topic_subscribe_BE_to_nello_learning                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_SUBSCRIPTION_TOPIC_BE_TO_NELLO_ACK        		target_to_store_nello_topic_subscribe_ACK_From_BE                   /**< MQTT topic to which this application subscribes. */

#define APP_MQTT_PUBLISH_TOPIC_SM         						target_to_store_nello_topic_publish_test                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_PUBLISH_TOPIC_NELLO_ONLINE        				target_to_store_nello_topic_publish_online                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_PUBLISH_TOPIC_NELLO_TO_BE         				target_to_store_nello_topic_publish_nello_to_BE_learning                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_PUBLISH_TOPIC_NELLO_ACK         				target_to_store_nello_topic_publish_ACK                   /**< MQTT topic to which this application subscribes. */
#define APP_MQTT_PUBLISH_TOPIC_NELLO_USER_MAPPING         		target_to_store_nello_topic_publish_user_mapping                   /**< MQTT topic to which this application subscribes. */


static const char        m_device_id[] = NELLO_DEVICE_ID_USED_FOR_MQTT_TOPICS;  




char * nello_topic_prefix	= "/nello_one/";

//Subscribe
char * nello_topic_suffix_subscribe_ACK_from_BE				=	"/BE_ACK/";
char * nello_topic_suffix_subscribe_test					=	"/test/";
char * nello_topic_suffix_subscribe_BE_to_nello_learning 	=	"/BEn/";
char * nello_topic_suffix_subscribe_straight_door_open		=	"/door/";
char * nello_topic_suffix_subscribe_geofence				=	"/geo/";
char * nello_topic_suffix_subscribe_time_window				=	"/tw/";

//Publish
char * nello_topic_suffix_publish_test						=	"/ring/";
char * nello_topic_suffix_publish_online					=	"/n_online/";
char * nello_topic_suffix_publish_nello_to_BE_learning		=	"/n_to_BE/";
char * nello_topic_suffix_publish_ACK						=	"/n_ACK/";
char * nello_topic_suffix_publish_user_mapping				=	"/map/";


char * nello_device_id = NELLO_DEVICE_ID_USED_FOR_MQTT_TOPICS;

char target_to_store_nello_topic_subscribe_test[30];
char target_to_store_nello_topic_subscribe_BE_to_nello_learning[30];
char target_to_store_nello_topic_subscribe__straight_door_open[30];
char target_to_store_nello_topic_subscribe_geofence[30];
char target_to_store_nello_topic_subscribe_time_window[30];
char target_to_store_nello_topic_subscribe_ACK_From_BE[30];

char target_to_store_nello_topic_publish_test[30];
char target_to_store_nello_topic_publish_online[30];
char target_to_store_nello_topic_publish_nello_to_BE_learning[30];
char target_to_store_nello_topic_publish_ACK[30];
char target_to_store_nello_topic_publish_user_mapping[30];


flag_do_the_mqtt_publish_in_main = 1;
		message_content_for_mqtt_publish_in_main = "1";
		topic_name_for_mqtt_publish_in_main = (unsigned char *) APP_MQTT_PUBLISH_TOPIC_NELLO_TO_BE;
		
		
		
		//Publsih Topics
	sprintf(target_to_store_nello_topic_publish_test,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_publish_test);
	sprintf(target_to_store_nello_topic_publish_online,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_publish_online);
	sprintf(target_to_store_nello_topic_publish_nello_to_BE_learning,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_publish_nello_to_BE_learning);
	sprintf(target_to_store_nello_topic_publish_ACK,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_publish_ACK);
	sprintf(target_to_store_nello_topic_publish_user_mapping,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_publish_user_mapping);
	
	
	//Subscribed Topics
	sprintf(target_to_store_nello_topic_subscribe_test,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_test);
	sprintf(target_to_store_nello_topic_subscribe_BE_to_nello_learning,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_BE_to_nello_learning);
	sprintf(target_to_store_nello_topic_subscribe__straight_door_open,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_straight_door_open);
	sprintf(target_to_store_nello_topic_subscribe_geofence,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_geofence);
	sprintf(target_to_store_nello_topic_subscribe_time_window,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_time_window);
	sprintf(target_to_store_nello_topic_subscribe_ACK_From_BE,"%s%s%s",nello_topic_prefix,nello_device_id,nello_topic_suffix_subscribe_ACK_from_BE);