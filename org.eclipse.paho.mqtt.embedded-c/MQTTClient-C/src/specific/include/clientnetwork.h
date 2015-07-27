#ifndef MQTT_NETWORK
#define MQTT_NETWORK
typedef struct Network Network;
struct Network
{
        void *physical_medium;
        int (*mqttread) (Network*, unsigned char*, int);
        int (*mqttwrite) (Network*, unsigned char*, int);
        void (*disconnect) (Network*);
};

Network* get_new_network();
void release_network(Network*);
#endif
